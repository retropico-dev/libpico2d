//
// Created by cpasjuste on 30/05/23.
//

#ifndef PICO_PSRAM_RP2040

#include <cstdio>
#include <pico/multicore.h>
#include <hardware/clocks.h>
#include "platform.h"
#include "pico_display.h"

#include "st7789.h"

using namespace p2d;

static PicoDisplay *s_display;

static Surface **s_surfaces;
static Utility::Vec2i s_display_size;
static Utility::Vec4i s_render_bounds;
// TODO: free scale_row_buffer ?
static uint16_t * __attribute__((aligned(4))) scale_row_buffer;

// core1 stuff
#define CORE1_CMD_FLIP 0
#define CORE1_CMD_EXIT 1

union core_cmd {
    struct {
        uint8_t cmd;
        uint8_t fb;
    };

    uint32_t full;
};

static core_cmd cmd_flip{.cmd = CORE1_CMD_FLIP};
static core_cmd cmd_exit{.cmd = CORE1_CMD_EXIT};
static int core1_busy = 0;
static bool s_core1_started = false;

static void in_ram(core1_main)();

static void in_ram(draw)(Surface *surface);

static void in_ram(scale_buffer_bilinear)(uint16_t *pixels, uint32_t pitch, uint32_t bpp,
                                          uint16_t src_w, uint16_t src_h,
                                          uint16_t dst_w, uint16_t dst_h);

PicoDisplay::PicoDisplay(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                         const Utility::Vec4i &renderBounds, const Buffering &buffering,
                         const Format &format, float spiSpeedMhz)
    : Display(displaySize, renderSize, renderBounds, buffering, format, spiSpeedMhz) {
    // init st7789 display
    st7789_init(m_format == Format::RGB565 ? ST7789_COLOR_MODE_16BIT : ST7789_COLOR_MODE_12BIT, spiSpeedMhz);

    // handle alpha channel removal (st7789 support rgb444)
    if (m_format == Format::ARGB444) m_bit_shift = 4;

    // my own core1 crap
    s_display = this;
    s_display_size = m_displaySize;
    s_render_bounds = m_renderBounds;
    s_surfaces = p_surfaces;

    // alloc frames buffers
    p_surfaces[0] = new Surface(getSize());
    if (m_buffering == Buffering::Double) {
        p_surfaces[1] = new Surface(getSize());
        // launch core1
#if defined(PICO_DEBUG_UART)
        multicore_reset_core1(); // seems to be needed for "picoprobe" debugging
#endif
        multicore_launch_core1(core1_main);
        printf("PicoDisplay: st7789 pio with double buffering @ %i Mhz (%ix%i => %ix%i)\r\n",
               (uint16_t) spiSpeedMhz, m_renderSize.x, m_renderSize.y, m_renderBounds.w, m_renderBounds.h);
    } else {
        printf("PicoDisplay: st7789 pio with single buffering @ %i Mhz (%ix%i => %i%i)\r\n",
               (uint16_t) spiSpeedMhz, m_renderSize.x, m_renderSize.y, m_renderBounds.w, m_renderBounds.h);
    }

    // clear display
    st7789_clear();
}

void PicoDisplay::setCursor(int16_t x, int16_t y) {
    m_cursor = {x, y};
}

void PicoDisplay::setDisplayBounds(int16_t x, int16_t y, uint16_t w, uint16_t h) {
    m_cursor = {x, y};
    st7789_set_cursor(x, y, w, h);
}

__always_inline void PicoDisplay::put(uint16_t color) {
    bool clip = m_cursor.x < m_clip.x || m_cursor.y < m_clip.y
                || m_cursor.x >= m_clip.x + m_clip.w || m_cursor.y >= m_clip.y + m_clip.h;
    if (!clip && color != m_colorKey && m_cursor.x < m_renderSize.x && m_cursor.y < m_renderSize.y) {
        *(uint16_t *) (p_surfaces[m_bufferIndex]->getPixels() + m_cursor.y * m_pitch + m_cursor.x * m_bpp)
                = color << m_bit_shift;
    }

    // emulate tft lcd "put_pixel"
    m_cursor.x++;
    if (m_cursor.x >= m_renderSize.x) {
        m_cursor.x = 0;
        m_cursor.y += 1;
    }
}

__always_inline void PicoDisplay::putFast(const uint16_t *buffer, uint32_t count) {
    memcpy(p_surfaces[m_bufferIndex]->getPixels() + m_cursor.y * m_pitch + m_cursor.x * m_bpp, buffer, count * m_bpp);
}

__always_inline void PicoDisplay::flush() {
    st7789_flush();
}

__always_inline void PicoDisplay::clear() {
    const auto buffer = p_surfaces[m_bufferIndex]->getPixels();
    if (m_clearColor == Black || m_clearColor == White) {
        memset(buffer, m_clearColor, p_surfaces[m_bufferIndex]->getPixelsSize());
    } else {
        const int size = m_pitch * m_renderBounds.h;
        uint64_t color64 = static_cast<uint64_t>(m_clearColor) << 48;
        color64 |= static_cast<uint64_t>(m_clearColor) << 32;
        color64 |= static_cast<uint64_t>(m_clearColor) << 16;
        color64 |= m_clearColor;
        for (uint_fast16_t i = 0; i < size; i += 8) {
            *reinterpret_cast<uint64_t *>(buffer + i) = color64;
        }
    }
}

void in_ram(PicoDisplay::flip)() {
    if (m_buffering == Buffering::Double) {
        // wait for previous buffer flip if needed
        while (__atomic_load_n(&core1_busy, __ATOMIC_SEQ_CST)) {
        }

        // send core1 flip "cmd"
        cmd_flip.fb = m_bufferIndex;
        __atomic_store_n(&core1_busy, 1, __ATOMIC_SEQ_CST);
        multicore_fifo_push_blocking(cmd_flip.full);

        // flip buffers
        m_bufferIndex = !m_bufferIndex;
    } else {
        // slow...
        draw(p_surfaces[m_bufferIndex]);
        st7789_flush();
    }
}

// crappy hack to "pause" core1 while writing to flash,
// I was not able to use "multicore_lockout_*" without problems

void in_ram(p2d_display_pause)() {
    //printf("p2d_display_pause\r\n");
    if (s_core1_started) {
        while (__atomic_load_n(&core1_busy, __ATOMIC_SEQ_CST)) {
        }
        __atomic_store_n(&core1_busy, 1, __ATOMIC_SEQ_CST);
        multicore_fifo_push_blocking(cmd_exit.full);
        while (__atomic_load_n(&core1_busy, __ATOMIC_SEQ_CST)) {
        }
    }
    //printf("p2d_display_pause: true\r\n");
}

void in_ram(p2d_display_resume)() {
    //printf("p2d_display_resume\r\n");
    if (s_core1_started) {
        multicore_reset_core1();
        multicore_launch_core1(core1_main);
    }
    //printf("p2d_display_resume: true\r\n");
}

static void in_ram(core1_main)() {
    core_cmd cmd{};
    s_core1_started = true;

    while (true) {
        cmd.full = multicore_fifo_pop_blocking();
        if (cmd.cmd == CORE1_CMD_FLIP) {
            st7789_flush();
            draw(s_surfaces[cmd.fb]);
        } else {
            break;
        }

        __atomic_store_n(&core1_busy, 0, __ATOMIC_SEQ_CST);
    }

    st7789_flush();
    __atomic_store_n(&core1_busy, 0, __ATOMIC_SEQ_CST);
}

static void in_ram(draw)(Surface *surface) {
    const auto surfaceSize = surface->getSize();
    const auto pixels = surface->getPixels();
    const auto pitch = s_display->getPitch();
    const auto bpp = s_display->getBpp();
    const auto maxWidth = std::min(surfaceSize.x, (int16_t) (s_display_size.x - s_render_bounds.x));
    const auto maxHeight = std::min(surfaceSize.y, (int16_t) (s_display_size.y - s_render_bounds.y));
    if (maxWidth <= 0 || maxHeight <= 0) return;

    if (surfaceSize == s_display_size) {
        // render size is equal to display size (fastest)
        st7789_set_cursor(0, 0, s_display_size.x, s_display_size.y);
        st7789_push(reinterpret_cast<const uint16_t *>(pixels), s_display_size.x * s_display_size.y);
        return;
    }

    if (surfaceSize == Utility::Vec2i(s_render_bounds.w, s_render_bounds.h)) {
        // surface size is equal to render size, draw without scaling  (fastest)
        st7789_set_cursor(s_render_bounds.x, s_render_bounds.y, maxWidth, maxHeight);
        st7789_push(reinterpret_cast<const uint16_t *>(pixels), maxWidth * maxHeight);
        return;
    }

    if (s_render_bounds.w / surfaceSize.x == 2 && s_render_bounds.h / surfaceSize.y == 2) {
        st7789_set_cursor(0, 0);
        // scale2x
        for (uint_fast16_t y = 0; y < maxHeight; y++) {
            for (uint_fast16_t i = 0; i < 2; i++) {
                for (uint_fast16_t x = 0; x < maxWidth; x += 2) {
                    // line 1
                    const auto p1 = *reinterpret_cast<uint16_t *>(pixels + y * pitch + x * bpp);
                    st7789_put16(p1);
                    st7789_put16(p1);
                    // line 2
                    const auto p2 = *reinterpret_cast<uint16_t *>(pixels + y * pitch + (x + 1) * bpp);
                    st7789_put16(p2);
                    st7789_put16(p2);
                }
            }
        }

        return;
    }

#if 0
    // bilinear scaling (rp2350)
    scale_buffer_bilinear((uint16_t *) pixels, pitch, bpp,
                          surfaceSize.x, surfaceSize.y,
                          s_render_bounds.w, s_render_bounds.h);
#else
    // nearest-neighbor scaling
    uint_fast16_t x, y;
    uint_fast16_t xRatio = (surfaceSize.x << 16) / s_render_bounds.w + 1;
    uint_fast16_t yRatio = (surfaceSize.y << 16) / s_render_bounds.h + 1;
    st7789_set_cursor(0, 0);
    for (uint_fast16_t i = 0; i < s_render_bounds.h; i++) {
        for (uint_fast16_t j = 0; j < s_render_bounds.w; j++) {
            x = (j * xRatio) >> 16;
            y = (i * yRatio) >> 16;
            st7789_put16(*(uint16_t *) (pixels + y * pitch + x * bpp));
        }
    }
#endif
}

// bilinear interpolation scaling
void in_ram(scale_buffer_bilinear)(uint16_t *pixels, uint32_t pitch, uint32_t bpp,
                                   uint16_t src_w, uint16_t src_h,
                                   uint16_t dst_w, uint16_t dst_h) {
    const uint32_t x_ratio = ((src_w - 1) << 16) / dst_w;
    const uint32_t y_ratio = ((src_h - 1) << 16) / dst_h;
    const uint32_t pixels_per_row = pitch / bpp; // Convert pitch to pixel units

    // Allocate row buffer for batch writing
    if (!scale_row_buffer) {
        scale_row_buffer = static_cast<uint16_t *>(malloc(dst_w * sizeof(uint16_t)));
    }

    if (!scale_row_buffer) return;

    st7789_set_cursor(0, 0);

    for (uint16_t i = 0; i < dst_h; i++) {
        uint32_t y = (i * y_ratio) >> 16;
        uint32_t y_diff = (i * y_ratio) & 0xFFFF;
        uint32_t y_next = (y + 1 < src_h) ? y + 1 : y;

        // calculate row offsets in pixels
        const uint32_t offset_y1 = y * pixels_per_row;
        const uint32_t offset_y2 = y_next * pixels_per_row;

        for (uint16_t j = 0; j < dst_w; j++) {
            uint32_t x = (j * x_ratio) >> 16;
            uint32_t x_diff = (j * x_ratio) & 0xFFFF;
            uint32_t x_next = (x + 1 < src_w) ? x + 1 : x;

            // get the four surrounding pixels using proper array indexing
            uint16_t p1 = pixels[offset_y1 + x];
            uint16_t p2 = pixels[offset_y1 + x_next];
            uint16_t p3 = pixels[offset_y2 + x];
            uint16_t p4 = pixels[offset_y2 + x_next];

            // extract RGB components (RGB565 format)
            uint32_t r1 = (p1 >> 11) & 0x1F;
            uint32_t g1 = (p1 >> 5) & 0x3F;
            uint32_t b1 = p1 & 0x1F;

            uint32_t r2 = (p2 >> 11) & 0x1F;
            uint32_t g2 = (p2 >> 5) & 0x3F;
            uint32_t b2 = p2 & 0x1F;

            uint32_t r3 = (p3 >> 11) & 0x1F;
            uint32_t g3 = (p3 >> 5) & 0x3F;
            uint32_t b3 = p3 & 0x1F;

            uint32_t r4 = (p4 >> 11) & 0x1F;
            uint32_t g4 = (p4 >> 5) & 0x3F;
            uint32_t b4 = p4 & 0x1F;

            // calculate interpolation weights (0-256)
            uint32_t x_weight = (x_diff >> 8);
            uint32_t y_weight = (y_diff >> 8);
            uint32_t x_weight_inv = 256 - x_weight;
            uint32_t y_weight_inv = 256 - y_weight;

            // interpolate each color component with fixed-point math
            uint32_t r = ((r1 * x_weight_inv + r2 * x_weight) * y_weight_inv +
                          (r3 * x_weight_inv + r4 * x_weight) * y_weight) >> 16;

            uint32_t g = ((g1 * x_weight_inv + g2 * x_weight) * y_weight_inv +
                          (g3 * x_weight_inv + g4 * x_weight) * y_weight) >> 16;

            uint32_t b = ((b1 * x_weight_inv + b2 * x_weight) * y_weight_inv +
                          (b3 * x_weight_inv + b4 * x_weight) * y_weight) >> 16;

            // pack RGB components back into RGB565
            scale_row_buffer[j] = ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
            //st7789_put16(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F));
        }

        // write the entire row
        st7789_push(scale_row_buffer, dst_w);
    }
}

#endif
