//
// Created by cpasjuste on 30/05/23.
//

#include <cstdio>
#include "platform.h"
#include "pico_display.h"
#include "st7789.h"

using namespace mb;

PicoDisplay::PicoDisplay() : Display({DISPLAY_WIDTH, DISPLAY_HEIGHT}) {
    printf("PicoDisplay: st7789 pio @ %ix%i\r\n", m_size.x, m_size.y);

    // init st7789 display
    st7789_init();

    // clear the display
    PicoDisplay::clear();
}

void PicoDisplay::setCursorPos(int16_t x, int16_t y) {
    if (x >= 0 && x < m_size.x && y >= 0 && y < m_size.y) {
        st7789_set_cursor(x, y);
    }
}

void PicoDisplay::setPixel(uint16_t color) {
    if (color != m_colorKey) st7789_put(color);
}
