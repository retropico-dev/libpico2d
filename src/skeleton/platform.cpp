//
// Created by cpasjuste on 29/12/23.
//

#include "platform.h"

using namespace mb;

static Platform *s_platform = nullptr;


Platform::Platform(bool useDoubleBufferDisplay, bool overclock) : Rectangle({}, Display::Color::Transparent) {
    s_platform = this;
}

bool Platform::loop(bool forceDraw) {
    uint16_t buttons = p_input->getButtons();
    if (buttons & Input::Button::QUIT) return false;

    // only refresh screen on button inputs
    if (forceDraw || buttons && buttons != Input::Button::DELAY) {
        // clear screen
        p_display->clear();

        // draw child's
        Rectangle::loop(m_position, buttons);

        // flip screen
        p_display->flip();
    }

    return true;
}

Platform *Platform::get() {
    return s_platform;
}
