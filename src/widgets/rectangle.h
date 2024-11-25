//
// Created by cpasjuste on 30/03/23.
//

#ifndef P2D_RECTANGLE_H
#define P2D_RECTANGLE_H

#include "widget.h"

namespace p2d {
    class Rectangle : public Widget {
    public:
        Rectangle(int16_t x, int16_t y, int16_t w, int16_t h,
                  uint16_t = Display::Color::White, int16_t radius = 0);

        Rectangle(const Utility::Vec2i &pos, const Utility::Vec2i &size,
                  uint16_t color = Display::Color::White, int16_t radius = 0);

        explicit Rectangle(const Utility::Vec4i &bounds,
                           uint16_t color = Display::Color::White, int16_t radius = 0);

        void onDraw(const Utility::Vec4i &bounds) override;

    private:
        int16_t m_radius = 0;
    };
}

#endif //P2D_RECTANGLE_H
