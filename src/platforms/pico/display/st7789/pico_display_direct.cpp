//
// Created by cpasjuste on 30/05/23.
//

#ifndef PICO_PSRAM_RP2040

#include <cstdio>
#include "platform.h"
#include "pico_display_direct.h"
#include "st7789.h"

using namespace p2d;

PicoDisplayDirectDraw::PicoDisplayDirectDraw(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                                             const Utility::Vec4i &renderBounds, const Buffering &buffering,
                                             const Format &format, float spiSpeedMhz)
    : Display(displaySize, renderSize, renderBounds, buffering, format, spiSpeedMhz) {
    // init st7789 display
    st7789_init(m_format == Format::RGB565 ? ST7789_COLOR_MODE_16BIT : ST7789_COLOR_MODE_12BIT, spiSpeedMhz);

    // handle alpha channel removal (st7789 support rgb444)
    if (m_format == Format::ARGB444) m_bit_shift = 4;

    printf("PicoDisplayDirectDraw: st7789 pio without buffering @ %i Mhz (%ix%i)\r\n",
           (uint16_t) spiSpeedMhz, renderBounds.w, renderBounds.h);
}

void PicoDisplayDirectDraw::setDisplayBounds(int16_t x, int16_t y, uint16_t w, uint16_t h) {
    st7789_set_cursor(x, y, w, h);
}

__always_inline void in_ram(PicoDisplayDirectDraw::setCursor)(int16_t x, int16_t y) {
    if (x >= 0 && x < m_renderBounds.w && y >= 0 && y < m_renderBounds.h) {
        st7789_set_cursor(x, y);
    }
}

__always_inline void in_ram(PicoDisplayDirectDraw::put)(uint16_t color) {
    // no alpha support (colorKey), prevent function call overhead in "direct drawing" mode
    st7789_put16(color << m_bit_shift);
}

__always_inline void in_ram(PicoDisplayDirectDraw::put)(const uint16_t *buffer, uint32_t count) {
    if (m_bit_shift == 0) {
        // RGB565
        st7789_push(buffer, count);
    } else {
        // ARGB444 > RGB444
        for (uint_fast16_t i = 0; i < count; i++) {
            st7789_put16(buffer[i] << m_bit_shift);
        }
    }
}

__always_inline void in_ram(PicoDisplayDirectDraw::putFast)(const uint16_t *buffer, uint32_t count) {
    if (m_bit_shift == 0) {
        // RGB565
        st7789_push(buffer, count);
    } else {
        // ARGB444 > RGB444
        for (uint_fast16_t i = 0; i < count; i++) {
            st7789_put16(buffer[i] << m_bit_shift);
        }
    }
}

__always_inline void in_ram(PicoDisplayDirectDraw::clear)() {
    st7789_set_cursor(0, 0);
    st7789_clear(m_clearColor);
}

__always_inline void in_ram(PicoDisplayDirectDraw::flush)() {
    st7789_flush();
}

#endif
