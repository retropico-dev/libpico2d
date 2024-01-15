//
// Created by cpasjuste on 30/05/23.
//

#include <cstdio>
#include <hardware/clocks.h>
#include "platform.h"
#include "pico_display_direct.h"
#include "st7789.h"

using namespace p2d;

static PicoDisplay *s_display;

static int s_core1_busy = 0;

static uint16_t in_ram(s_line_buffer)[DISPLAY_WIDTH];

static void in_ram(draw)(Surface *surface, bool scale,
                         const Utility::Vec2i &pos, const Utility::Vec2i &size);

_Noreturn static void in_ram(core1_main)();

union core_cmd {
    struct {
        uint8_t index = 0;
    };
    uint32_t full;
};

PicoDisplayDirectDraw::PicoDisplayDirectDraw(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                                             const Utility::Vec4i &renderBounds, const Buffering &buffering,
                                             const Format &format, float spiSpeedMhz)
        : Display(displaySize, renderSize, renderBounds, buffering, format, spiSpeedMhz) {
    // init st7789 display
    st7789_init(m_format == Format::RGB565 ?
                ST7789_COLOR_MODE_16BIT : ST7789_COLOR_MODE_12BIT, spiSpeedMhz);

    // handle alpha channel removal (st7789 support rgb444)
    if (m_format == Format::ARGB444) m_bit_shift = 4;

    printf("PicoDisplay: st7789 pio without buffering @ %i Mhz (%ix%i)\r\n",
           (uint16_t) spiSpeedMhz, renderBounds.w, renderBounds.h);

    // clear display
    PicoDisplayDirectDraw::clear();
}

__always_inline void PicoDisplayDirectDraw::setCursor(int16_t x, int16_t y) {
    if (x >= 0 && x < m_renderBounds.w && y >= 0 && y < m_renderBounds.h) {
        st7789_set_cursor(x, y);
    }
}

__always_inline void PicoDisplayDirectDraw::setPixel(uint16_t color) {
    // no alpha support (colorKey), prevent function call overhead in "direct drawing" mode
    st7789_put16(color << m_bit_shift);
}
