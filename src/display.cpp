//
// Created by cpasjuste on 05/06/23.
//

#include "platform.h"

using namespace p2d;

Display::Display(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                 const Utility::Vec4i &renderBounds, const Buffering &buffering,
                 const Format &format, float spiSpeedMhz)
        : Adafruit_GFX(renderBounds.w, renderBounds.h) {
    m_clip = {0, 0, renderBounds.x, renderBounds.y};
    m_displaySize = displaySize;
    m_renderSize = renderSize;
    m_renderBounds = renderBounds;
    m_buffering = buffering;
    m_format = format;
    m_pitch = m_renderSize.x * m_bpp;
    m_bpp = 2;

    // pixel line buffer for drawSurface
    m_line_buffer = (uint16_t *) malloc(m_pitch);
    memset(m_line_buffer, 0, m_pitch);
}

void in_ram(Display::put)(const uint16_t *buffer, uint32_t count) {
    for (uint_fast16_t i = 0; i < count; i++) {
        put(buffer[i]);
    }
}

void in_ram(Display::drawSurface)(Surface *surface, const Utility::Vec4i &bounds) {
    if (!surface) return;

    if (surface->getSize() == Utility::Vec2i{bounds.w, bounds.h}) { // no scaling
        auto isBitmap = surface->isBitmap();
        auto pixels = surface->getPixels();
        auto pitch = surface->getPitch();
        auto height = surface->getSize().y;
        auto width = std::min(surface->getSize().x, (int16_t) (m_renderSize.x - bounds.x));
        if (width <= 0) return;
        for (int16_t y = 0; y < bounds.h; y++) {
            // break if y is out of screen
            if (bounds.y + y < 0 || bounds.y + y >= m_renderSize.y) continue;
            // set cursor position
            setCursor(bounds.x, (int16_t) (bounds.y + y));
            // draw line
            if (isBitmap) {
                // invert Y axis
                if (surface->isAlphaEnabled())
                    put((uint16_t *) (pixels + (height - y - 1) * pitch), width);
                else
                    putFast((uint16_t *) (pixels + (height - y - 1) * pitch), width);
            } else {
                if (surface->isAlphaEnabled())
                    put((uint16_t *) (pixels + y * pitch), width);
                else
                    putFast((uint16_t *) (pixels + y * pitch), width);
            }
        }
    } else {
        // nearest-neighbor scaling
        int x, y;
        auto pitch = surface->getPitch();
        auto bpp = surface->getBpp();
        auto pixels = surface->getPixels();
        auto srcSize = surface->getSize();
        int xRatio = (srcSize.x << 16) / bounds.w + 1;
        int yRatio = (srcSize.y << 16) / bounds.h + 1;

        setCursor(bounds.x, bounds.y);

        for (uint8_t i = 0; i < bounds.h; i++) {
            for (uint8_t j = 0; j < bounds.w; j++) {
                x = (j * xRatio) >> 16;
                y = (i * yRatio) >> 16;
                if (x >= bounds.w) setCursor(bounds.x, (int16_t) (bounds.y + i));
                put(*(uint16_t *) (pixels + y * pitch + x * bpp));
            }
        }
    }
}

void in_ram(Display::clear)() {
    setCursor(0, 0);
    for (int y = 0; y < m_displaySize.y; y++) {
        for (int x = 0; x < m_displaySize.x; x++) {
            put(m_clearColor);
        }
    }
}

Display::~Display() {
    free(m_line_buffer);
}
