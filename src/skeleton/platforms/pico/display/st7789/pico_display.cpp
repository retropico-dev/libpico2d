//
// Created by cpasjuste on 30/05/23.
//

#include <cstdio>
#include "platform.h"
#include "pico_display.h"
#include "st7789.h"

using namespace mb;

PicoDisplay::PicoDisplay(const Utility::Vec2i &displaySize,
                         const Utility::Vec2i &renderSize,
                         const ScaleMode &scaleMode,
                         const Buffering &buffering)
        : Display(displaySize, renderSize, scaleMode, buffering) {
    printf("PicoDisplay: st7789 pio @ %ix%i (direct draw)\r\n", renderSize.x, renderSize.y);

    // init st7789 display
    st7789_init();

    // clear the display
    PicoDisplay::clear();
}

void PicoDisplay::setCursorPos(int16_t x, int16_t y) {
    m_cursor = {x, y};
    if (x >= 0 && x < m_renderSize.x && y >= 0 && y < m_renderSize.y) {
        st7789_set_cursor(x, y);
    }
}

void PicoDisplay::setPixel(uint16_t color) {
    if (color != m_colorKey) st7789_put(color);
    else {
        m_cursor.x++;
        if (m_cursor.x >= DISPLAY_WIDTH) {
            m_cursor.x = 0;
            m_cursor.y++;
        }
        st7789_set_cursor(m_cursor.x, m_cursor.y);
    }
}
