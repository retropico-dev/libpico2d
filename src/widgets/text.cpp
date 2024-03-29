//
// Created by cpasjuste on 30/03/23.
//

#include "platform.h"
#include "text.h"

using namespace p2d;

Text::Text(int16_t x, int16_t y, const std::string &str, Display::Color color) : Widget() {
    // default stuff
    Platform::instance()->getDisplay()->setTextWrap(false);

    Widget::setPosition(x, y);
    Widget::setSize(0, 0);
    m_color = color;
    setString(str);
}

Text::Text(int16_t x, int16_t y, int16_t w, int16_t h,
           const std::string &str, Display::Color color) : Widget() {
    // default stuff
    Platform::instance()->getDisplay()->setTextWrap(false);

    Widget::setPosition(x, y);
    Widget::setSize(w, h);
    m_color = color;
    setString(str);
}

Text::Text(const Utility::Vec2i &pos, const Utility::Vec2i &size,
           const std::string &str, Display::Color color) : Widget() {
    // default stuff
    Platform::instance()->getDisplay()->setTextWrap(false);

    Widget::setPosition(pos);
    Widget::setSize(size);
    m_color = color;
    setString(str);
}

std::string Text::getString() const {
    return m_text;
}

void Text::setString(const std::string &str) {
    m_text = str;
    Platform::instance()->getDisplay()->getTextBounds(m_text.c_str(), 0, 0,
                                                      &m_bounds.x, &m_bounds.y,
                                                      (uint16_t *) &m_bounds.w, (uint16_t *) &m_bounds.h);
    if (getSize().x == 0 || getSize().y == 0) {
        setSize(m_bounds.w, m_bounds.h);
    }
}

void Text::onDraw(const Utility::Vec2i &pos, bool draw) {
    if (!draw) return;

    // now draw the text
    Platform::instance()->getDisplay()->setTextColor(m_color);
    Platform::instance()->getDisplay()->setClipArea(
            {pos.x, pos.y, (int16_t) (pos.x + m_size.x), (int16_t) (pos.y + m_size.y)});
    Platform::instance()->getDisplay()->drawText(
            (int16_t) (pos.x - m_bounds.x), (int16_t) (pos.y - m_bounds.y), m_text);
    Platform::instance()->getDisplay()->setClipArea(
            {0, 0, Platform::instance()->getDisplay()->getSize().x,
             Platform::instance()->getDisplay()->getSize().y});

    // draw child's
    Widget::onDraw(pos, draw);
}
