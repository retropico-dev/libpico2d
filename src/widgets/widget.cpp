//
// Created by cpasjuste on 30/03/23.
//

#include <algorithm>
#include "platform.h"
#include "widget.h"

using namespace p2d;

void Widget::add(Widget *widget) {
    if (widget) {
        widget->p_parent = this;
        widget->updateBounds();
        p_childs.push_back(widget);
    }
}

void Widget::remove(Widget *widget) {
    if (!p_childs.empty()) {
        p_childs.erase(std::remove(
                           p_childs.begin(), p_childs.end(), widget), p_childs.end());
    }
}

void Widget::setPositionAndSize(const int16_t x, const int16_t y, const int16_t w, const int16_t h) {
    m_position.x = x;
    m_position.y = y;
    m_size.x = w;
    m_size.y = h;
    updateBounds();
}

void Widget::setPosition(const int16_t x, const int16_t y) {
    m_position.x = x;
    m_position.y = y;
    updateBounds();
}

void Widget::setPosition(const Utility::Vec2i &pos) {
    setPosition(pos.x, pos.y);
}

Utility::Vec2i Widget::getPosition() {
    return m_position;
}

void Widget::setSize(const int16_t x, const int16_t y) {
    m_size.x = x;
    m_size.y = y;
    updateBounds();
}

void Widget::setSize(const Utility::Vec2i &size) {
    setSize(size.x, size.y);
}

void Widget::updateBounds() {
    const Utility::Vec4i bounds =
            p_parent
                ? p_parent->m_bounds
                : Utility::Vec4i{0, 0, m_size.x, m_size.y};
    m_bounds = {
        static_cast<int16_t>(bounds.x + m_position.x),
        static_cast<int16_t>(bounds.y + m_position.y),
        m_size.x, m_size.y
    };

    switch (m_origin) {
        case Origin::Left:
            m_bounds.y -= m_size.y / 2;
            break;
        case Origin::Top:
            m_bounds.x -= m_size.x / 2;
            break;
        case Origin::TopRight:
            m_bounds.x -= m_size.x;
            break;
        case Origin::Right:
            m_bounds.x -= m_size.x;
            m_bounds.y -= m_size.y / 2;
            break;
        case Origin::BottomRight:
            m_bounds.x -= m_size.x;
            m_bounds.y -= m_size.y;
            break;
        case Origin::Bottom:
            m_bounds.x -= m_size.x / 2;
            m_bounds.y -= m_size.y;
            break;
        case Origin::BottomLeft:
            m_bounds.y -= m_size.y;
            break;
        case Origin::Center:
            m_bounds.x -= m_size.x / 2;
            m_bounds.y -= m_size.y / 2;
            break;
        case Origin::TopLeft:
        default:
            break;
    }
}

void Widget::onUpdate(Time delta) {
    for (auto &widget: p_childs) {
        if (widget) {
            widget->onUpdate(delta);
        }
    }
}

bool Widget::onInput(const uint16_t &buttons) {
    for (auto &widget: p_childs) {
        if (widget && widget->isVisible()) {
            if (widget->onInput(buttons)) {
                return true;
            }
        }
    }

    return false;
}

void Widget::onDraw(const bool draw) {
    if (!draw || m_visibility == Visibility::Hidden) {
        return;
    }

    for (const auto &widget: p_childs) {
        if (!widget->isVisible()) continue;
        widget->onDraw(draw);
    }
}

Widget::~Widget() {
    for (const auto widget = p_childs.begin(); widget != p_childs.end();) {
        if (*widget) {
            delete (*widget);
        }
    }
    p_childs.clear();
    if (p_parent) {
        p_parent->remove(this);
    }
}
