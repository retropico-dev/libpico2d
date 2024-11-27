//
// Created by cpasjuste on 30/03/23.
//

#ifndef U_GAMEPAD_TEXT_H
#define U_GAMEPAD_TEXT_H

#include "widget.h"
#include "tweeny.h"

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

        void setScroll(const bool enable) {
            m_do_tween = enable;
            m_tween_pos_x = 0;
            m_tween_elapsed_ms = 0;
        }

        void onUpdate(Time delta) override;

        void onDraw(const Utility::Vec4i &bounds) override;

    private:
        std::string m_text{};
        Utility::Vec4i m_text_bounds{};
        Utility::Vec2i m_max_size{};
        tweeny::tween<int16_t> m_tween{};
        uint16_t m_tween_pos_x = 0;
        uint32_t m_tween_start_delay_ms = 3000;
        uint32_t m_tween_elapsed_ms = 0;
        bool m_do_tween = false;
    };
}

#endif //U_GAMEPAD_TEXT_H
