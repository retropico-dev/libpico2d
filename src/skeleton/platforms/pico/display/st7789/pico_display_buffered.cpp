//
// Created by cpasjuste on 30/05/23.
//

#include <cstdio>
#include "platform.h"
#include "pico_display_buffered.h"
#include "st7789.h"

using namespace mb;

static int s_core1_busy = 0;

static PicoDisplayBuffered *s_display;

static uint16_t in_ram(s_line_buffer)[240];

static void in_ram(draw)(Surface *surface, const Display::ScaleMode &mode,
                         const Utility::Vec2i &pos, const Utility::Vec2i &size);

_Noreturn static void in_ram(core1_main)();

union core_cmd {
    struct {
        uint8_t index = 0;
    };
    uint32_t full;
};

PicoDisplayBuffered::PicoDisplayBuffered(const Utility::Vec2i &displaySize,
                                         const Utility::Vec2i &renderSize,
                                         const ScaleMode &scaleMode,
                                         const Buffering &buffering)
        : Display(displaySize, renderSize, scaleMode, buffering) {
    // my own core1 crap
    s_display = this;

    // init st7789 display
    st7789_init();

    // alloc frames buffers
    p_surfaces[0] = new Surface(m_renderSize);
    if (m_buffering == Buffering::Double) {
        p_surfaces[1] = new Surface(m_renderSize);
        // launch core1
        multicore_launch_core1(core1_main);
        printf("PicoDisplay: st7789 pio with double buffering @ %ix%i\r\n", m_renderSize.x, m_renderSize.y);
    } else {
        printf("PicoDisplay: st7789 pio with single buffering @ %ix%i\r\n", m_renderSize.x, m_renderSize.y);
    }

    // clear the display
    PicoDisplayBuffered::clear();
}

void in_ram(PicoDisplayBuffered::setCursorPos)(int16_t x, int16_t y) {
    m_cursor = {x, y};
}

void in_ram(PicoDisplayBuffered::setPixel)(uint16_t color) {
    if (color != m_colorKey) {
        p_surfaces[m_bufferIndex]->setPixel(m_cursor, color);
    }

    // emulate tft lcd "put_pixel"
    m_cursor.x++;
    if (m_cursor.x >= m_renderSize.x) {
        m_cursor.x = 0;
        m_cursor.y += 1;
    }
}

void in_ram(PicoDisplayBuffered::clear)(uint16_t color) {
    if (color == Color::Black || color == Color::White) {
        auto buffer = (uint16_t *) p_surfaces[m_bufferIndex]->getPixels();
        memset(buffer, color, p_surfaces[m_bufferIndex]->getPixelsSize());
    } else {
        auto buffer = p_surfaces[m_bufferIndex]->getPixels();
        int size = m_pitch * m_renderSize.y;
        uint64_t color64 = (uint64_t) color << 48;
        color64 |= (uint64_t) color << 32;
        color64 |= (uint64_t) color << 16;
        color64 |= color;
        for (uint_fast16_t i = 0; i < size; i += 8) {
            *(uint64_t *) (buffer + i) = color64;
        }
    }
}

void in_ram(PicoDisplayBuffered::flip)() {
    if (m_buffering == Buffering::Double) {
        // wait until previous flip on core1 complete
        while (__atomic_load_n(&s_core1_busy, __ATOMIC_SEQ_CST)) tight_loop_contents();
        // send "flip" cmd to core1
        __atomic_store_n(&s_core1_busy, 1, __ATOMIC_SEQ_CST);
        union core_cmd cmd{.index = m_bufferIndex};
        multicore_fifo_push_blocking(cmd.full);
        // flip buffer
        m_bufferIndex = !m_bufferIndex;
    } else {
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
                        st7789_put(mode == mb::Display::Point ? p1 : Display::Color::Black);
                        // line 2
                        auto p2 = *(uint16_t *) (pixels + y * pitch + (x + 1) * bpp);
                        st7789_put(p2);
                        st7789_put(mode == mb::Display::Point ? p2 : Display::Color::Black);
                    }
                }
            }
        }
    }
}
