//
// Created by cpasjuste on 14/06/23.
//

#ifndef PICO2D_UI_H
#define PICO2D_UI_H

#include <vector>
#include "rectangle.h"

#define UI_FONT_HEIGHT 16

namespace mb {
    class Ui : public Rectangle {
    public:
        enum Color {
            Black = 0x0000,
            White = 0xFFFF,
            Red = 0xC083,
            Yellow = 0xFF00,
            YellowLight = 0xF5C1,
            Green = 0x07E0,
            GreenDark = 0x0388,
            Blue = 0x01CF,
            Gray = 0x52CB,
            GrayDark = 0x2986,
            Transparent = 0xABCD // fake
        };

        explicit Ui(Platform *platform);

        bool loop(bool forceDraw = false);

        static Ui *getInstance();

        static Platform *getPlatform();

        static Display *getDisplay();

    private:
        Platform *p_platform;
        // auto-repeat
        uint16_t m_buttons_old = 0;
        Clock m_repeat_clock{};
    };
}

#endif //PICO2D_UI_H
