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

void Widget::setPosition(int16_t x, int16_t y) {
    m_position.x = x;
    m_position.y = y;
}

void Widget::setPosition(const Utility::Vec2i &pos) {
    setPosition(pos.x, pos.y);
}

Utility::Vec2i Widget::getPosition() {
    return m_position;
}

void Widget::setSize(int16_t x, int16_t y) {
    m_size.x = x;
    m_size.y = y;
}

void Widget::setSize(const Utility::Vec2i &size) {
    m_size.x = size.x;
    m_size.y = size.y;
}

Utility::Vec4i Widget::getBounds() {
    return {m_position.x, m_position.y, m_size.x, m_size.y};
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

void Widget::onDraw(const Utility::Vec2i &pos, bool draw) {
    if (!draw || m_visibility == Visibility::Hidden) {
        return;
    }

    for (auto &widget: p_childs) {
        if (!widget->isVisible()) continue;

        Utility::Vec2i v = {(int16_t) (pos.x + widget->m_position.x),
                            (int16_t) (pos.y + widget->m_position.y)};
        switch (widget->m_origin) {
            case Origin::Left:
                v.y -= widget->m_size.y / 2;
                break;
            case Origin::Top:
                v.x -= widget->m_size.x / 2;
                break;
            case Origin::TopRight:
                v.x -= widget->m_size.x;
                break;
            case Origin::Right:
                v.x -= widget->m_size.x;
                v.y -= widget->m_size.y / 2;
                break;
            case Origin::BottomRight:
                v.x -= widget->m_size.x;
                v.y -= widget->m_size.y;
                break;
            case Origin::Bottom:
                v.x -= widget->m_size.x / 2;
                v.y -= widget->m_size.y;
                break;
            case Origin::BottomLeft:
                v.y -= widget->m_size.y;
                break;
            case Origin::Center:
                v.x -= widget->m_size.x / 2;
                v.y -= widget->m_size.y / 2;
                break;
            case Origin::TopLeft:
            default:
                break;
        }

        widget->onDraw(v, draw);
    }
}

Widget::~Widget() {
    for (auto widget = p_childs.begin(); widget != p_childs.end();) {
        if (*widget) {
            delete (*widget);
        }
    }
    p_childs.clear();
    if (p_parent) {
        p_parent->remove(this);
    }
}
