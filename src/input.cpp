//
// Created by cpasjuste on 31/05/23.
//

#include "input.h"

using namespace p2d;

void Input::onUpdate() {
    // save "raw" buttons state
    m_raw_buttons = m_buttons;

    // handle auto-repeat
    if (m_repeatClock.getElapsedTime().asMilliseconds() >= m_repeatDelayMs) {
        m_repeatClock.restart();
        m_buttons_prev = m_buttons;
    } else {
        uint16_t diff = m_buttons_prev ^ m_buttons;
        m_buttons_prev = m_buttons;
        if (diff > 0) {
            m_repeatClock.restart();
        } else {
            m_buttons = Button::DELAY;
        }
    }
}
