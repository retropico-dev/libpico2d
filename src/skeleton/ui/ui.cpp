//
// Created by cpasjuste on 14/06/23.
//

#include "platform.h"
#include "ui.h"

using namespace mb;

//...
static Ui *s_ui = nullptr;

Ui::Ui(Platform *platform) : Rectangle({0, 0}, platform->getDisplay()->getSize(), Color::Transparent) {
    s_ui = this;
    p_platform = platform;

    // set repeat delay for ui
    p_platform->getInput()->setRepeatDelay(INPUT_DELAY_UI);
}

bool Ui::loop(bool forceDraw) {
    uint16_t buttons = p_platform->getInput()->getButtons();
    if (buttons & Input::Button::QUIT) return false;

    // handle auto-repeat speed
    if (buttons != Input::Button::DELAY) {
        bool changed = (m_buttons_old ^ buttons) != 0;
        m_buttons_old = buttons;
        if (!changed) {
            if (m_repeat_clock.getElapsedTime().asSeconds() > 6) {
                p_platform->getInput()->setRepeatDelay(INPUT_DELAY_UI / 20);
            } else if (m_repeat_clock.getElapsedTime().asSeconds() > 4) {
                p_platform->getInput()->setRepeatDelay(INPUT_DELAY_UI / 8);
            } else if (m_repeat_clock.getElapsedTime().asSeconds() > 2) {
                p_platform->getInput()->setRepeatDelay(INPUT_DELAY_UI / 4);
            }
        } else {
            p_platform->getInput()->setRepeatDelay(INPUT_DELAY_UI);
            m_repeat_clock.restart();
        }
    }

    // only refresh screen on button inputs
    if (forceDraw || buttons && buttons != Input::Button::DELAY) {
        // clear screen
        p_platform->getDisplay()->clear();

        // draw child's
        Rectangle::loop(m_position, buttons);

        // flip screen
        p_platform->getDisplay()->flip();
    }

    return true;
}

Ui *Ui::getInstance() {
    return s_ui;
}

Platform *Ui::getPlatform() {
    return s_ui->p_platform;
}

Display *Ui::getDisplay() {
    return s_ui->p_platform->getDisplay();
}
