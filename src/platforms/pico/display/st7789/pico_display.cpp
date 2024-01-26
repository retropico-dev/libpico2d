//
// Created by cpasjuste on 30/05/23.
//

#include <cstdio>
#include <hardware/clocks.h>
#include "platform.h"
#include "pico_display.h"
#include "st7789.h"

using namespace p2d;

static PicoDisplay *s_display;

static Surface **s_surfaces;
static Utility::Vec2i s_display_size;
static Utility::Vec4i s_render_bounds;
static uint8_t s_bit_shift;

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

static void in_ram(core1_main)();

static void in_ram(draw)(Surface *surface);

PicoDisplay::PicoDisplay(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                         const Utility::Vec4i &renderBounds, const Buffering &buffering,
                         const Format &format, float spiSpeedMhz)
        : Display(displaySize, renderSize, renderBounds, buffering, format, spiSpeedMhz) {
    // init st7789 display
    st7789_init(m_format == Format::RGB565 ?
                ST7789_COLOR_MODE_16BIT : ST7789_COLOR_MODE_12BIT, spiSpeedMhz);

    // handle alpha channel removal (st7789 support rgb444)
    if (m_format == Format::ARGB444) s_bit_shift = m_bit_shift = 4;

    // my own core1 crap
    s_display = this;
    s_display_size = m_displaySize;
    s_render_bounds = m_renderBounds;
    s_surfaces = p_surfaces;

    // alloc frames buffers
    p_surfaces[0] = new Surface(PicoDisplay::getSize());
    if (m_buffering == Buffering::Double) {
        p_surfaces[1] = new Surface(PicoDisplay::getSize());
        // launch core1
#if defined(PICO_DEBUG_UART)
        multicore_reset_core1(); // seems to be needed for "picoprobe" debugging
#endif
        multicore_launch_core1(core1_main);
        printf("PicoDisplay: st7789 pio with double buffering @ %i Mhz (%ix%i => %ix%i) (pio: 0, sm: 0)\r\n",
               (uint16_t) spiSpeedMhz, m_renderSize.x, m_renderSize.y, m_renderBounds.w, m_renderBounds.h);
    } else {
        printf("PicoDisplay: st7789 pio with single buffering @ %i Mhz (%ix%i => %i%i) (pio: 0, sm: 0)\r\n",
               (uint16_t) spiSpeedMhz, m_renderSize.x, m_renderSize.y, m_renderBounds.w, m_renderBounds.h);
    }

    // clear display
    clearDisplay();
}

void PicoDisplay::setCursor(int16_t x, int16_t y) {
    m_cursor = {x, y};
}

__always_inline void PicoDisplay::setPixel(uint16_t color) {
    if (color != m_colorKey && m_cursor.x < m_renderSize.x && m_cursor.y < m_renderSize.y) {
        *(uint16_t *) (p_surfaces[m_bufferIndex]->getPixels() + m_cursor.y * m_pitch + m_cursor.x * m_bpp) = color;
    }
    // emulate tft lcd "put_pixel"
    m_cursor.x++;
    if (m_cursor.x >= m_renderSize.x) {
        m_cursor.x = 0;
        m_cursor.y += 1;
    }
}

void PicoDisplay::clearDisplay() {
    st7789_set_cursor(0, 0);
    for (uint_fast16_t y = 0; y < m_displaySize.x; y++) {
        for (uint_fast16_t x = 0; x < m_displaySize.y; x++) {
            st7789_put16(Display::Color::Black);
        }
    }
}

void in_ram(PicoDisplay::clear)() {
    auto buffer = p_surfaces[m_bufferIndex]->getPixels();
    if (m_clearColor == Color::Black || m_clearColor == Color::White) {
        memset((uint16_t *) buffer, m_clearColor, p_surfaces[m_bufferIndex]->getPixelsSize());
    } else {
        int size = m_pitch * m_renderBounds.h;
        uint64_t color64 = (uint64_t) m_clearColor << 48;
        color64 |= (uint64_t) m_clearColor << 32;
        color64 |= (uint64_t) m_clearColor << 16;
        color64 |= m_clearColor;
        for (uint_fast16_t i = 0; i < size; i += 8) {
            *(uint64_t *) (buffer + i) = color64;
        }
    }
}

void in_ram(PicoDisplay::flip)() {
    if (m_buffering == Buffering::Double) {
        // wait for previous buffer flip if needed
        while (__atomic_load_n(&core1_busy, __ATOMIC_SEQ_CST)) {
            //printf("PicoDisplay::flip: waiting for rendering done...\r\n");
            tight_loop_contents();
        }

        // send core1 flip "cmd"
        cmd_flip.fb = m_bufferIndex;
        __atomic_store_n(&core1_busy, 1, __ATOMIC_SEQ_CST);
        multicore_fifo_push_blocking(cmd_flip.full);

        // flip buffers
        m_bufferIndex = !m_bufferIndex;
    } else if (m_buffering == Buffering::Single) {
        // slow...
        draw(p_surfaces[m_bufferIndex]);
    }
}

// crappy hack to pause core1 while writing to flash,
// I was not able to use "multicore_lockout_victim_init" without random crashes
static bool core1_stop = false;
static bool core1_stopped = true;

void in_ram(p2d_display_pause)() {
    //printf("p2d_display_pause\r\n");

    if (!core1_stopped) {
        core1_stop = true;
        multicore_fifo_push_blocking(cmd_exit.full);
        while (!core1_stopped) tight_loop_contents();
    }

    //printf("p2d_display_pause: true\r\n");
}

void in_ram(p2d_display_resume)() {
    //printf("p2d_display_resume\r\n");

    if (core1_stop) {
        multicore_reset_core1();
        multicore_launch_core1(core1_main);
    }

    //printf("p2d_display_resume: true\r\n");
}

static void in_ram(core1_main)() {
    core_cmd cmd{};

    //multicore_lockout_victim_init();
    core1_stop = false;
    core1_stopped = false;

    while (!core1_stop) {
        cmd.full = multicore_fifo_pop_blocking();
        if (cmd.cmd == CORE1_CMD_FLIP) {
            draw(s_surfaces[cmd.fb]);
        }

        __atomic_store_n(&core1_busy, 0, __ATOMIC_SEQ_CST);
#if 0
        while (!s_flip) tight_loop_contents();
        draw(s_surfaces[s_buffer_index]);
        s_flip = false;
#endif
    }

    core1_stopped = true;
}

static void in_ram(draw)(Surface *surface) {
    auto surfaceSize = surface->getSize();
    auto pixels = surface->getPixels();
    auto pitch = s_display->getPitch();
    auto bpp = s_display->getBpp();
    auto maxWidth = std::min(surfaceSize.x, (int16_t) (s_display_size.x - s_render_bounds.x));
    auto maxHeight = std::min(surfaceSize.y, (int16_t) (s_display_size.y - s_render_bounds.y));
    if (maxWidth <= 0 || maxHeight <= 0) return;

    // set initial cursor position
    st7789_set_cursor(s_render_bounds.x, s_render_bounds.y);

    if (surfaceSize == s_display_size) {
        // render size is equal to display size, fastest
        for (int_fast16_t y = 0; y < maxHeight; y++) {
            for (int_fast16_t x = 0; x < maxWidth; x++) {
                st7789_put16(*(uint16_t *) (pixels + y * pitch + x * bpp) << s_bit_shift);
            }
        }

        return;
    }

    if (surfaceSize == Utility::Vec2i(s_render_bounds.w, s_render_bounds.h)) {
        // surface size is equal to render size, draw without scaling
        for (int_fast16_t y = 0; y < maxHeight; y++) {
            for (int_fast16_t x = 0; x < maxWidth + 1; x++) {
                if (x == maxWidth) {
                    st7789_set_cursor(s_render_bounds.x, (int16_t) (s_render_bounds.y + y));
                    continue;
                }
                st7789_put16(*(uint16_t *) (pixels + y * pitch + x * bpp) << s_bit_shift);
            }
        }

        return;
    }

    if (s_render_bounds.w / surfaceSize.x == 2 && s_render_bounds.h / surfaceSize.y == 2) {
        // scale2x
        for (uint_fast16_t y = 0; y < maxHeight; y++) {
            for (uint_fast16_t i = 0; i < 2; i++) {
                for (uint_fast16_t x = 0; x < maxWidth; x += 2) {
                    // line 1
                    auto p1 = *(uint16_t *) (pixels + y * pitch + x * bpp) << s_bit_shift;
                    st7789_put16(p1);
                    st7789_put16(p1);
                    // line 2
                    auto p2 = *(uint16_t *) (pixels + y * pitch + (x + 1) * bpp) << s_bit_shift;
                    st7789_put16(p2);
                    st7789_put16(p2);
                }
            }
        }

        return;
    }

    // nearest-neighbor scaling
    uint_fast16_t x, y;
    uint_fast16_t xRatio = (surfaceSize.x << 16) / s_render_bounds.w + 1;
    uint_fast16_t yRatio = (surfaceSize.y << 16) / s_render_bounds.h + 1;
    for (uint_fast16_t i = 0; i < s_render_bounds.h; i++) {
        for (uint_fast16_t j = 0; j < s_render_bounds.w; j++) {
            x = (j * xRatio) >> 16;
            y = (i * yRatio) >> 16;
            st7789_put16(*(uint16_t *) (pixels + y * pitch + x * bpp) << s_bit_shift);
        }
    }
}
