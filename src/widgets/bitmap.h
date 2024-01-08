//
// Created by cpasjuste on 30/03/23.
//

#ifndef U_MICROBOY_BITMAP_H
#define U_MICROBOY_BITMAP_H

#include "widget.h"

namespace p2d {
    class Bitmap : public Widget {
    public:
        explicit Bitmap(Surface *surface, const Utility::Vec2i &pos = {});

        explicit Bitmap(const uint8_t *bmp, const Utility::Vec2i &pos = {});

        ~Bitmap();

        void loop(const Utility::Vec2i &pos, const uint16_t &buttons) override;

    private:
        Surface *m_surface = nullptr;
    };
}

#endif //U_MICROBOY_BITMAP_H
