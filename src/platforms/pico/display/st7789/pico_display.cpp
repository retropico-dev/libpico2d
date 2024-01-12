//
// Created by cpasjuste on 30/05/23.
//

#include <cstdio>
#include <hardware/clocks.h>
#include "platform.h"
#include "pico_display.h"
#include "st7789.h"

using namespace p2d;

#ifndef PICO_DISPLAY_DIRECT_DRAW

static int s_core1_busy = 0;

static PicoDisplay *s_display;

static uint16_t in_ram(s_line_buffer)[DISPLAY_WIDTH];

static void in_ram(draw)(Surface *surface, const Display::ScaleMode &mode,
                         const Utility::Vec2i &pos, const Utility::Vec2i &size);

_Noreturn static void in_ram(core1_main)();

union core_cmd {
    struct {
        uint8_t index = 0;
    };
    uint32_t full;
};

#endif

PicoDisplay::PicoDisplay(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                         const Buffering &buffering, const ScaleMode &scaleMode, const Format &format)
        : Display(displaySize, renderSize, buffering, scaleMode, format) {
    // init st7789 display
    // 62.5f = pico default @ 125 Mhz sys clock (safe)
    // tested working at 88.6 Mhz @ 266 Mhz clock (unsafe)
    auto spi_clock = 62.5f;
    auto sys_clock = (uint16_t) (clock_get_hz(clk_sys) / 1000000);
    auto clock_div = (float) sys_clock * (62.5f / spi_clock) / 125;
    st7789_init(m_format == Format::RGB565 ? ST7789_COLOR_MODE_16bit : ST7789_COLOR_MODE_12bit, clock_div);
    if (m_format == Format::ARGB444) m_bit_shift = 4;

#ifdef PICO_DISPLAY_DIRECT_DRAW
    printf("PicoDisplay: st7789 pio without buffering @ %i Mhz (%ix%i)\r\n",
           (uint16_t) spi_clock, renderSize.x, renderSize.y);
#else
    // my own core1 crap
    s_display = this;

    // alloc frames buffers
    p_surfaces[0] = new Surface(m_renderSize);
    if (m_buffering == Buffering::Double) {
        p_surfaces[1] = new Surface(m_renderSize);
        // launch core1
#if !defined(NDEBUG) && defined(PICO_STDIO_UART)
        multicore_reset_core1(); // seems to be needed for "picoprobe" debugging
#endif
        multicore_launch_core1(core1_main);
        printf("PicoDisplay: st7789 pio with double buffering @ %i Mhz (%ix%i)\r\n",
               (uint16_t) spi_clock, m_renderSize.x, m_renderSize.y);
    } else {
        printf("PicoDisplay: st7789 pio with single buffering @ %i Mhz (%ix%i)\r\n",
               (uint16_t) spi_clock, m_renderSize.x, m_renderSize.y);
    }
#endif

    // clear display buffers
    PicoDisplay::clear();
    PicoDisplay::flip();
    PicoDisplay::clear();
}

void PicoDisplay::setCursor(int16_t x, int16_t y) {
#ifdef PICO_DISPLAY_DIRECT_DRAW
    if (x >= 0 && x < m_renderSize.x && y >= 0 && y < m_renderSize.y) {
        st7789_set_cursor(x, y);
    }
#else
    m_cursor = {x, y};
#endif
}

void PicoDisplay::setPixel(uint16_t color) {
#ifdef PICO_DISPLAY_DIRECT_DRAW
    // no alpha support, prevent slowdown in "direct drawing" mode
    st7789_put16(color << m_bit_shift);
#else
    if (color != m_colorKey && m_cursor.x < m_renderSize.x && m_cursor.y < m_renderSize.y) {
        *(uint16_t *) (p_surfaces[m_bufferIndex]->getPixels() + m_cursor.y * m_pitch + m_cursor.x * m_bpp) = color;
    }
    // emulate tft lcd "put_pixel"
    m_cursor.x++;
    if (m_cursor.x >= m_displaySize.x) {
        m_cursor.x = 0;
        m_cursor.y += 1;
    }
#endif
}

#ifndef PICO_DISPLAY_DIRECT_DRAW

void in_ram(PicoDisplay::clear)() {
    auto buffer = p_surfaces[m_bufferIndex]->getPixels();
    if (m_clearColor == Color::Black || m_clearColor == Color::White) {
        memset((uint16_t *) buffer, m_clearColor, p_surfaces[m_bufferIndex]->getPixelsSize());
    } else {
        int size = m_pitch * m_renderSize.y;
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
        // wait until previous flip on core1 complete
        while (__atomic_load_n(&s_core1_busy, __ATOMIC_SEQ_CST)) tight_loop_contents();
        // send "flip" cmd to core1
        __atomic_store_n(&s_core1_busy, 1, __ATOMIC_SEQ_CST);
        union core_cmd cmd{.index = m_bufferIndex};
        multicore_fifo_push_blocking(cmd.full);
        // flip buffer
        m_bufferIndex = !m_bufferIndex;
    } else if (m_buffering == Buffering::Single) {
        draw(p_surfaces[m_bufferIndex], m_scaleMode, {}, m_displaySize);
    }
}

_Noreturn static void in_ram(core1_main)() {
    union core_cmd cmd{};

    while (true) {
        cmd.full = multicore_fifo_pop_blocking();
        draw(s_display->getSurface(cmd.index), s_display->getScaleMode(), {0, 0}, s_display->getDisplaySize());
        __atomic_store_n(&s_core1_busy, 0, __ATOMIC_SEQ_CST);
    }
}

static void in_ram(draw)(Surface *surface, const Display::ScaleMode &mode,
                         const Utility::Vec2i &pos, const Utility::Vec2i &size) {
    // render size is equal to display size
    if (surface->getSize() == size) {
        auto buffer = surface->getPixels();
        uint16_t pitch = s_display->getPitch();
        uint8_t bpp = s_display->getBpp();
        st7789_set_cursor(0, 0);
        for (uint_fast16_t y = 0; y < size.y; y++) {
            for (uint_fast16_t x = 0; x < size.x; x++) {
                st7789_put(*(uint16_t *) (buffer + y * pitch + x * bpp));
            }
        }
    } else {
        auto pixels = surface->getPixels();
        auto pitch = surface->getPitch();
        auto bpp = surface->getBpp();
        auto srcSize = surface->getSize();

        st7789_set_cursor(pos.x, pos.y);

        if (mode == Display::ScaleMode::Nearest) {
            int x, y;
            int xRatio = (srcSize.x << 16) / size.x + 1;
            int yRatio = (srcSize.y << 16) / size.y + 1;
            for (uint8_t i = 0; i < size.y; i++) {
                // computer line
                for (uint8_t j = 0; j < size.x; j++) {
                    x = (j * xRatio) >> 16;
                    y = (i * yRatio) >> 16;
                    s_line_buffer[j + pos.x] = *(uint16_t *) (pixels + y * pitch + x * bpp);
                }
                // render line
                if (size.x == DISPLAY_WIDTH) {
                    for (uint_fast16_t k = 0; k < size.x; k++) {
                        st7789_put(s_line_buffer[k]);
                    }
                } else {
                    st7789_set_cursor(pos.x, i + pos.y);
                    for (uint_fast16_t k = 0; k < size.x; k++) {
                        st7789_put(s_line_buffer[k]);
                    }
                }
            }
        } else {
            for (uint_fast16_t y = 0; y < srcSize.y; y++) {
                for (uint_fast16_t i = 0; i < 2; i++) {
                    for (uint_fast16_t x = 0; x < srcSize.x; x += 2) {
                        // line 1
                        auto p1 = *(uint16_t *) (pixels + y * pitch + x * bpp);
                        st7789_put(p1);
                        st7789_put(mode == p2d::Display::Scale2x ? p1 : Display::Color::Black);
                        // line 2
                        auto p2 = *(uint16_t *) (pixels + y * pitch + (x + 1) * bpp);
                        st7789_put(p2);
                        st7789_put(mode == p2d::Display::Scale2x ? p2 : Display::Color::Black);
                    }
                }
            }
        }
    }
}

#endif