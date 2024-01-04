//
// Created by cpasjuste on 30/03/23.
//

#ifndef U_MICROBOY_BITMAP_H
#define U_MICROBOY_BITMAP_H

#include "widget.h"

namespace mb {
    class Bitmap : public Widget {
    public:
        Bitmap(Surface *surface, const Utility::Vec2i &pos = {});

        void loop(const Utility::Vec2i &pos, const uint16_t &buttons) override;

    private:
        Surface *m_surface = nullptr;
    };
}

#endif //U_MICROBOY_BITMAP_H
