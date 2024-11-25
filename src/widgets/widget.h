//
// Created by cpasjuste on 30/03/23.
//

#ifndef U_GAMEPAD_WIDGET_H
#define U_GAMEPAD_WIDGET_H

#include <cstdint>
#include <vector>
#include "display.h"
#include "utility.h"

namespace p2d {
    class Widget {
    public:
        enum class Origin : uint8_t {
            Left, TopLeft, Top, TopRight,
            Right, BottomRight, Bottom, BottomLeft, Center
        };

        enum class Visibility : uint8_t {
            Visible, Hidden
        };

        Widget() = default;

        Widget(const Utility::Vec2i &pos, const Utility::Vec2i &size) {
            m_position = pos;
            m_size = size;
            m_color = Display::Color::Black;
        }

        explicit Widget(const Utility::Vec4i &bounds) {
            m_position = {bounds.x, bounds.y};
            m_size = {bounds.w, bounds.h};
            m_color = Display::Color::Black;
        }

        virtual ~Widget();

        virtual void add(Widget *widget);

        virtual void remove(Widget *widget);

        virtual void setPositionAndSize(int16_t x, int16_t y, int16_t w, int16_t h);

        virtual void setPosition(int16_t x, int16_t y);

        virtual void setPosition(const Utility::Vec2i &pos);

        virtual Utility::Vec2i getPosition();

        virtual void setSize(int16_t x, int16_t y);

        virtual void setSize(const Utility::Vec2i &size);

        virtual Utility::Vec2i getSize() { return m_size; }

        virtual Utility::Vec4i getBounds() { return m_bounds; }

        virtual void setOrigin(const Origin &origin) {
            m_origin = origin;
        }

        virtual Origin getOrigin() { return m_origin; }

        virtual void setColor(const uint16_t color) { m_color = color; }

        virtual uint16_t getColor() { return m_color; }

        virtual void setOutlineColor(const uint16_t color) { m_outline_color = color; }

        virtual uint16_t getOutlineColor() { return m_outline_color; }

        virtual void setOutlineThickness(const uint16_t size) { m_outline_thickness = size; }

        virtual uint16_t getOutlineThickness() { return m_outline_thickness; }

        virtual bool isVisible() { return m_visibility == Visibility::Visible; }

        virtual void setVisibility(const Visibility visibility) { m_visibility = visibility; }

        virtual Widget *getParent() { return p_parent; }

        virtual void onUpdate(Time delta);

        virtual bool onInput(const uint16_t &buttons);

        virtual void onDraw(const Utility::Vec4i &bounds);

    protected:
        Widget *p_parent = nullptr;
        std::vector<Widget *> p_childs;
        Visibility m_visibility = Visibility::Visible;
        Utility::Vec2i m_position{};
        Utility::Vec2i m_size{};
        Origin m_origin = Origin::TopLeft;
        uint16_t m_color = Display::Color::White;
        uint16_t m_outline_color = Display::Color::Red;
        uint16_t m_outline_thickness = 0;

    private:
        Utility::Vec4i m_bounds{};
    };
}

#endif //U_GAMEPAD_WIDGET_H
