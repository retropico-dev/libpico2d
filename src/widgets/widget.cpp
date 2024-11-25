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
    m_position.x = m_bounds.x = x;
    m_position.y = m_bounds.y = y;
    m_size.x = m_bounds.w = w;
    m_size.y = m_bounds.h = h;
}

void Widget::setPosition(const int16_t x, const int16_t y) {
    m_position.x = m_bounds.x = x;
    m_position.y = m_bounds.y = y;
}

void Widget::setPosition(const Utility::Vec2i &pos) {
    setPosition(pos.x, pos.y);
}

Utility::Vec2i Widget::getPosition() {
    return m_position;
}

void Widget::setSize(const int16_t x, const int16_t y) {
    m_size.x = m_bounds.w = x;
    m_size.y = m_bounds.h = y;
}

void Widget::setSize(const Utility::Vec2i &size) {
    setSize(size.x, size.y);
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

void Widget::onDraw(const Utility::Vec4i &bounds) {
    if (m_visibility == Visibility::Hidden) {
        return;
    }

    for (const auto &widget: p_childs) {
        if (!widget->isVisible()) continue;

        Utility::Vec4i b = {
            static_cast<int16_t>(bounds.x + widget->m_position.x),
            static_cast<int16_t>(bounds.y + widget->m_position.y),
            widget->m_bounds.w, widget->m_bounds.h
        };

        switch (widget->m_origin) {
            case Origin::Left:
                b.y -= widget->m_size.y / 2;
                break;
            case Origin::Top:
                b.x -= widget->m_size.x / 2;
                break;
            case Origin::TopRight:
                b.x -= widget->m_size.x;
                break;
            case Origin::Right:
                b.x -= widget->m_size.x;
                b.y -= widget->m_size.y / 2;
                break;
            case Origin::BottomRight:
                b.x -= widget->m_size.x;
                b.y -= widget->m_size.y;
                break;
            case Origin::Bottom:
                b.x -= widget->m_size.x / 2;
                b.y -= widget->m_size.y;
                break;
            case Origin::BottomLeft:
                b.y -= widget->m_size.y;
                break;
            case Origin::Center:
                b.x -= widget->m_size.x / 2;
                b.y -= widget->m_size.y / 2;
                break;
            case Origin::TopLeft:
            default:
                break;
        }

        widget->onDraw(b);
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
