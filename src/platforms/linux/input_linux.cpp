//
// Created by cpasjuste on 31/05/23.
//

#include "platform.h"

using namespace p2d;

struct Mapping {
    uint16_t button;
    uint8_t scancode;
};

const Mapping mapping[MAX_BUTTONS] = {
        {Input::Button::B1,       SDL_SCANCODE_KP_1},
        {Input::Button::B2,       SDL_SCANCODE_KP_2},
        {Input::Button::START,    SDL_SCANCODE_KP_4},
        {Input::Button::SELECT,   SDL_SCANCODE_KP_5},
        {Input::Button::LEFT,     SDL_SCANCODE_LEFT},
        {Input::Button::RIGHT,    SDL_SCANCODE_RIGHT},
        {Input::Button::UP,       SDL_SCANCODE_UP},
        {Input::Button::DOWN,     SDL_SCANCODE_DOWN},
        {Input::Button::VOL_UP,   SDL_SCANCODE_KP_PLUS},
        {Input::Button::VOL_DOWN, SDL_SCANCODE_KP_MINUS}
};

void LinuxInput::onUpdate() {
    // reset buttons state
    m_buttons = 0;

    // check for quit event
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            m_buttons = Input::Button::QUIT;
            return;
        }
    }

    // check for buttons (keyboard) press
    for (const auto &map: mapping) {
        m_buttons |= SDL_GetKeyboardState(nullptr)[map.scancode] > 0 ? map.button : 0;
    }

    // handle repeat delay
    Input::onUpdate();
}
