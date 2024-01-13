//
// Created by cpasjuste on 05/06/23.
//

#include "platform.h"

using namespace p2d;

Display::Display(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                 const Buffering &buffering, const ScaleMode &scaleMode,
                 const Format &format, float spiSpeedMhz)
        : Adafruit_GFX(renderSize.x, renderSize.y) {
    m_clip = {0, 0, renderSize.x, renderSize.y};
    m_displaySize = displaySize;
    m_renderSize = renderSize;
    m_scaleMode = scaleMode;
    m_format = format;
    m_buffering = buffering;
    m_pitch = m_renderSize.x * m_bpp;
    m_bpp = 2;

    // pixel line buffer for drawSurface
    m_line_buffer = (uint16_t *) malloc(m_pitch);
    memset(m_line_buffer, 0, m_pitch);
}

// very slow, obviously...
void in_ram(Display::drawPixel)(int16_t x, int16_t y, uint16_t color) {
    if ((x < m_clip.x) || (y < m_clip.y)
        || x >= m_clip.x + m_clip.w || y >= m_clip.y + m_clip.h)
        return;

    if (rotation) {
        int16_t t;
        switch (rotation) {
            case 1:
                t = x;
                x = (int16_t) (m_renderSize.x - 1 - y);
                y = t;
                break;
            case 2:
                x = (int16_t) (m_renderSize.x - 1 - x);
                y = (int16_t) (m_renderSize.y - 1 - y);
                break;
            case 3:
                t = x;
                x = y;
                y = (int16_t) (m_renderSize.y - 1 - t);
                break;
            default:
                break;
        }
    }

    setCursor(x, y);
    setPixel(color);
}

// faster
void in_ram(Display::drawPixelLine)(const uint16_t *pixels, uint16_t width) {
    for (uint_fast16_t i = 0; i < width; i++) {
        setPixel(pixels[i]);
    }
}

void in_ram(Display::drawSurface)(Surface *surface, const Utility::Vec2i &pos, const Utility::Vec2i &dstSize) {
    if (!surface) return;

    if (surface->getSize() == dstSize) { // no scaling
        auto isBitmap = surface->isBitmap();
        auto pixels = surface->getPixels();
        auto pitch = surface->getPitch();
        auto height = surface->getSize().y;
        auto width = std::min(surface->getSize().x, (int16_t) (m_renderSize.x - pos.x));
        for (int16_t y = 0; y < dstSize.y; y++) {
            // skip horizontal lines if out of screen
            if (pos.y + y < 0 || pos.y + y >= m_renderSize.y) continue;
            // set cursor position
            setCursor(pos.x, (int16_t) (pos.y + y));
            // draw line
            if (isBitmap) // invert Y axis
                drawPixelLine((uint16_t *) (pixels + (height - y - 1) * pitch), width);
            else
                drawPixelLine((uint16_t *) (pixels + y * pitch), width);
        }
    } else {
        // nearest-neighbor scaling
        int x, y;
        auto pitch = surface->getPitch();
        auto bpp = surface->getBpp();
        auto pixels = surface->getPixels();
        auto srcSize = surface->getSize();
        int xRatio = (srcSize.x << 16) / dstSize.x + 1;
        int yRatio = (srcSize.y << 16) / dstSize.y + 1;

        setCursor(pos.x, pos.y);

        for (uint8_t i = 0; i < dstSize.y; i++) {
            for (uint8_t j = 0; j < dstSize.x; j++) {
                x = (j * xRatio) >> 16;
                y = (i * yRatio) >> 16;
                if (x >= dstSize.x) setCursor(pos.x, (int16_t) (pos.y + i));
                setPixel(*(uint16_t *) (pixels + y * pitch + x * bpp));
            }
        }
    }
}

void in_ram(Display::clear)() {
    setCursor(0, 0);
    for (int y = 0; y < m_displaySize.y; y++) {
        for (int x = 0; x < m_displaySize.x; x++) {
            setPixel(m_clearColor);
        }
    }
}

Display::~Display() {
    free(m_line_buffer);
}
