//
// Created by cpasjuste on 30/03/23.
//

#include "platform.h"
#include "text.h"

using namespace p2d;

Text::Text(const int16_t x, const int16_t y,
           const std::string &str, const Display::Color color) : Widget() {
    // default stuff
    Platform::instance()->getDisplay()->setTextWrap(false);
    Widget::setPositionAndSize(x, y, 0, 0);
    m_color = color;
    setString(str);
}

Text::Text(const int16_t x, const int16_t y, const int16_t w, const int16_t h,
           const std::string &str, const Display::Color color) : Widget() {
    // default stuff
    Platform::instance()->getDisplay()->setTextWrap(false);
    Widget::setPositionAndSize(x, y, w, h);
    m_max_size = {w, h};
    m_color = color;
    setString(str);
}

Text::Text(const Utility::Vec2i &pos, const Utility::Vec2i &size,
           const std::string &str, Display::Color color) : Widget() {
    // default stuff
    Platform::instance()->getDisplay()->setTextWrap(false);
    Widget::setPositionAndSize(pos.x, pos.y, size.x, size.y);
    m_max_size = size;
    m_color = color;
    setString(str);
}

std::string Text::getString() const {
    return m_text;
}

void Text::setString(const std::string &str) {
    m_text = str;
    Platform::instance()->getDisplay()->getTextBounds(
        m_text.c_str(), 0, 0, &m_text_bounds.x, &m_text_bounds.y,
        reinterpret_cast<uint16_t *>(&m_text_bounds.w), reinterpret_cast<uint16_t *>(&m_text_bounds.h));
    if (m_max_size.x != 0 || m_max_size.y != 0) {
        setSize(m_text_bounds.w, m_text_bounds.h);
    } else {
        setSize(m_max_size);
    }
}

void Text::onUpdate(const Time delta) {
    if (m_do_tween && m_max_size.x < m_text_bounds.w) {
        const auto progress = m_tween.progress();
        m_tween_elapsed_ms += delta.asMilliseconds();

        if (m_tween_elapsed_ms > m_tween_start_delay_ms) {
            if (m_tween_pos_x == 0) {
                // start tween
                const auto diff = m_text_bounds.w - m_max_size.x;
                const auto dst_x = static_cast<int16_t>(diff - m_position.x + 2);
                m_tween = tweeny::from(m_position.x).to(dst_x).during(3 * 1000);
                m_tween_pos_x = m_tween.step(delta.asMilliseconds(), true);
            } else if (progress <= 0.0f || progress >= 1.0f) {
                if (m_tween_elapsed_ms > m_tween.duration() + m_tween_start_delay_ms + 2000) {
                    // restart tween
                    m_tween.seek(0);
                    m_tween_pos_x = 0;
                    m_tween_elapsed_ms = 0.0f;
                }
            } else {
                // step tween
                m_tween_pos_x = m_tween.step(delta.asMilliseconds(), true);
            }
        }
    }

    Widget::onUpdate(delta);
}

void Text::onDraw(const Utility::Vec4i &bounds) {
    // now draw the text
    Platform::instance()->getDisplay()->setTextColor(m_color);
    Platform::instance()->getDisplay()->setClipArea({bounds.x, bounds.y, m_max_size.x, m_max_size.y});
    Platform::instance()->getDisplay()->drawText(static_cast<int16_t>(bounds.x - m_tween_pos_x), bounds.y, m_text);
    Platform::instance()->getDisplay()->setClipArea(
        {
            0, 0,
            Platform::instance()->getDisplay()->getSize().x,
            Platform::instance()->getDisplay()->getSize().y
        });

    // draw child's
    Widget::onDraw(bounds);
}
