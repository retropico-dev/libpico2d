//
// Created by cpasjuste on 29/12/23.
//

#include "platform.h"

using namespace p2d;

static Platform *s_platform = nullptr;

Platform::Platform() : Widget() {
    s_platform = this;
}

Platform *Platform::instance() {
    return s_platform;
}

bool Platform::loop(bool forceDraw) {
    uint16_t buttons = p_input ? p_input->getButtons() : 0;
    if (buttons & Input::Button::QUIT) return false;

    // only refresh screen on button inputs
    if (p_display && (forceDraw || buttons && buttons != Input::Button::DELAY)) {
        // clear screen
        p_display->clear();

        // draw child's
        Widget::loop(m_position, buttons);

        // flip screen
        p_display->flip();
    }

    return true;
}

Platform::~Platform() {
    printf("~Platform()\n");
    delete (p_display);
    delete (p_audio);
    delete (p_input);
    Io::exit();
}
