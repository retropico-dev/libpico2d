//
// Created by cpasjuste on 30/03/23.
//

#ifndef P2D_BITMAP_H
#define P2D_BITMAP_H

#include "widget.h"

namespace p2d {
    class Bitmap final : public Widget {
    public:
        explicit Bitmap(Surface *surface, const Utility::Vec2i &pos = {});

        explicit Bitmap(const Io::File &file, const Utility::Vec2i &pos = {});

        ~Bitmap() override;

        void onDraw(bool draw) override;

        Surface *getSurface() { return m_surface; }

        [[nodiscard]] bool isAlphaEnabled() const {
            if (m_surface) return m_surface->isAlphaEnabled();
            return false;
        }

        void setAlphaEnabled(const bool value) const {
            if (m_surface) m_surface->setAlphaEnabled(value);
        }

    private:
        Surface *m_surface = nullptr;
        bool m_read_only = false;
    };
}

#endif //P2D_BITMAP_H
