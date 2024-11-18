//
// Created by cpasjuste on 30/03/23.
//

#ifndef U_GAMEPAD_TEXT_H
#define U_GAMEPAD_TEXT_H

#include "widget.h"
#include "rectangle.h"

namespace p2d {
    class Text : public Widget {
    public:
        Text(int16_t x, int16_t y,
             const std::string &text, Display::Color color = Display::Color::White);

        Text(int16_t x, int16_t y, int16_t w, int16_t h,
             const std::string &text, Display::Color color = Display::Color::White);

        Text(const Utility::Vec2i &pos, const Utility::Vec2i &size,
             const std::string &text, Display::Color color = Display::Color::White);

        [[nodiscard]] std::string getString() const;

        void setString(const std::string &str);

        void onDraw(bool draw) override;

    private:
        std::string m_text{};
    };
}

#endif //U_GAMEPAD_TEXT_H
