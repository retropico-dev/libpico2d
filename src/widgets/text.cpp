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
    m_color = color;
    setString(str);
}

Text::Text(const Utility::Vec2i &pos, const Utility::Vec2i &size,
           const std::string &str, Display::Color color) : Widget() {
    // default stuff
    Platform::instance()->getDisplay()->setTextWrap(false);
    Widget::setPositionAndSize(pos.x, pos.y, size.x, size.y);
    m_color = color;
    setString(str);
}

std::string Text::getString() const {
    return m_text;
}

void Text::setString(const std::string &str) {
    m_text = str;
    Utility::Vec4i bounds{};
    Platform::instance()->getDisplay()->getTextBounds(
        m_text.c_str(), 0, 0, &bounds.x, &bounds.y,
        reinterpret_cast<uint16_t *>(&bounds.w), reinterpret_cast<uint16_t *>(&bounds.h));
    setSize(bounds.w, bounds.h);
}

void Text::onDraw(const bool draw) {
    if (!draw) return;

    // now draw the text
    Platform::instance()->getDisplay()->setTextColor(m_color);
    Platform::instance()->getDisplay()->setClipArea({m_bounds.x, m_bounds.y, m_bounds.w, m_bounds.y});
    Platform::instance()->getDisplay()->drawText(m_bounds.x, m_bounds.y, m_text);
    Platform::instance()->getDisplay()->setClipArea(
        {
            0, 0,
            Platform::instance()->getDisplay()->getSize().x,
            Platform::instance()->getDisplay()->getSize().y
        });

    // draw child's
    Widget::onDraw(draw);
}
