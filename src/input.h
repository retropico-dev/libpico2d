//
// Created by cpasjuste on 31/05/23.
//

#ifndef P2D_INPUT_H
#define P2D_INPUT_H

#include <cstdint>
#include <string>
#include "clock.h"

#ifndef BIT
#define BIT(n) (1U<<(n))
#endif

#define MAX_BUTTONS 10
#define INPUT_DELAY_UI 300

namespace p2d {
    class Input {
    public:
        enum Button {
            B1 = BIT(0),
            B2 = BIT(1),
            START = BIT(2),
            SELECT = BIT(3),
            LEFT = BIT(4),
            RIGHT = BIT(5),
            UP = BIT(6),
            DOWN = BIT(7),
            VOL_UP = BIT(8),
            VOL_DOWN = BIT(9),
            DELAY = BIT(10),
            QUIT = BIT(11)
        };

        struct Mapping {
            uint16_t button{};
            int8_t pin{};
            std::string name{};
        };

        virtual void onUpdate();

        virtual uint16_t getButtons() { return m_buttons; }

        uint16_t getRepeatDelay() { return m_repeatDelayMs; };

        void setRepeatDelay(uint16_t ms) { m_repeatDelayMs = ms; };

        Mapping *getMapping() { return m_mapping; };

    protected:
        Clock m_repeatClock{};
        uint16_t m_buttons{0};
        uint16_t m_buttons_prev{0};
        uint16_t m_repeatDelayMs = 0;
        Mapping m_mapping[MAX_BUTTONS];
    };
}

#endif //P2D_INPUT_H
