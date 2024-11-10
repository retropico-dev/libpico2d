//
// Created by cpasjuste on 31/05/23.
//

#include "platform.h"
#include "pinout.h"

#if defined(BTN_PIN_SLEEP)

#include "sleep.h"

#endif

using namespace p2d;

PicoInput::PicoInput() : Input() {
    // setup buttons pins
    m_mapping[0] = {B1, BTN_PIN_A, "B1"};
    m_mapping[1] = {B2, BTN_PIN_B, "B2"};
    m_mapping[2] = {START, BTN_PIN_START, "START"};
    m_mapping[3] = {SELECT, BTN_PIN_SELECT, "SELECT"};
    m_mapping[4] = {LEFT, BTN_PIN_LEFT, "LEFT"};
    m_mapping[5] = {RIGHT, BTN_PIN_RIGHT, "RIGHT"};
    m_mapping[6] = {UP, BTN_PIN_UP, "UP"};
    m_mapping[7] = {DOWN, BTN_PIN_DOWN, "DOWN"};
#if defined(BTN_PIN_VOL_U) && defined(BTN_PIN_VOL_D)
    m_mapping[8] = {VOL_UP, BTN_PIN_VOL_U, "VOL_UP"};
    m_mapping[9] = {VOL_DOWN, BTN_PIN_VOL_D, "VOL_DOWN"};
#endif

    for (const auto &map: m_mapping) {
        if (map.pin != -1) {
            gpio_set_function(map.pin, GPIO_FUNC_SIO);
            gpio_set_dir(map.pin, GPIO_IN);
            gpio_pull_up(map.pin);
            printf("PicoInput: setting up pin %i as %s (pull-up: %i)\r\n",
                   map.pin, map.name.c_str(), gpio_is_pulled_up(map.pin));
        }
    }

#if defined(BTN_PIN_SLEEP)
    gpio_set_function(BTN_PIN_SLEEP, GPIO_FUNC_SIO);
    gpio_set_dir(BTN_PIN_SLEEP, false);
    gpio_pull_up(BTN_PIN_SLEEP);
    printf("PicoInput: setting up pin %i as sleep button\r\n", BTN_PIN_SLEEP);
#endif
}

void PicoInput::onUpdate() {
    // reset buttons state
    m_buttons = 0;

    // check for buttons (keyboard) press
    for (const auto &map: m_mapping) {
        if (map.pin != -1) m_buttons |= gpio_get(map.pin) ? 0 : map.button;
    }

#if defined(BTN_PIN_SLEEP)
    // check for sleep button
    if (!gpio_get(BTN_PIN_SLEEP)) {
        Sleep::sleep();
    }
#endif

#if defined(PICO_DEBUG_UART) || defined(PICO_DEBUG_USB)
    int c = getchar_timeout_us(0);
    switch (c) {
        case 49: // NUMPAD 1
            m_buttons |= B1;
            break;
        case 50: // NUMPAD 2
            m_buttons |= B2;
            break;
        case 52: // NUMPAD 4
            m_buttons |= SELECT;
            break;
        case 53: // NUMPAD 5
            m_buttons |= START;
            break;
        case 65: // ARROW UP
            m_buttons |= UP;
            break;
        case 66: // ARROW DOWN
            m_buttons |= DOWN;
            break;
        case 67: // ARROW RIGHT
            m_buttons |= RIGHT;
            break;
        case 68: // ARROW LEFT
            m_buttons |= LEFT;
            break;
        case 'q':
            m_buttons |= QUIT;
            break;
        default:
            break;
    }
#endif

    // handle repeat delay
    Input::onUpdate();
}
