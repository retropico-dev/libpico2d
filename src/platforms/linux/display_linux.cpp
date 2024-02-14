//
// Created by cpasjuste on 30/05/23.
//

#include <cstdio>
#include "platform.h"
#include "display_linux.h"

using namespace p2d;

LinuxDisplay::LinuxDisplay(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                           const Utility::Vec4i &renderBounds, const Buffering &buffering,
                           const Format &format, float spiSpeedMhz)
        : Display(displaySize, renderSize, renderBounds, buffering, format, spiSpeedMhz) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s", SDL_GetError());
        return;
    }

    p_window = SDL_CreateWindow("libpico2d", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                m_displaySize.x, m_displaySize.y, SDL_WINDOW_SHOWN);
    if (!p_window) {
        fprintf(stderr, "SDL_CreateWindow error: %s", SDL_GetError());
        return;
    }

    p_renderer = SDL_CreateRenderer(p_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!p_renderer) {
        fprintf(stderr, "SDL_CreateRenderer error: %s", SDL_GetError());
        return;
    }

    if (m_renderSize != Utility::Vec2i(m_renderBounds.w, m_renderBounds.h)) {
        SDL_RenderSetLogicalSize(p_renderer, m_renderSize.x, m_renderSize.y);
    }

    p_surfaces[0] = new Surface(m_renderSize);

    // handle alpha channel removal (st7789 support rgb444)
    if (m_format == Format::ARGB444) m_bit_shift = 4;

    printf("LinuxDisplay: %ix%i (texture pitch: %i)\n", m_displaySize.x, m_displaySize.y, m_pitch);
}

void LinuxDisplay::setCursor(int16_t x, int16_t y) {
    m_cursor = {x, y};
}

void LinuxDisplay::put(uint16_t color) {
    bool clip = m_cursor.x < m_clip.x || m_cursor.y < m_clip.y
                || m_cursor.x >= m_clip.x + m_clip.w || m_cursor.y >= m_clip.y + m_clip.h;
    if (!clip && color != m_colorKey && m_cursor.x < m_renderSize.x && m_cursor.y < m_renderSize.y) {
        *(uint16_t *) (p_surfaces[0]->getPixels() + m_cursor.y * m_pitch + m_cursor.x * m_bpp)
                = color << m_bit_shift;
    }

    // emulate tft lcd "put_pixel"
    m_cursor.x++;
    if (m_cursor.x >= m_renderSize.x) {
        m_cursor.x = 0;
        m_cursor.y += 1;
    }
}

void LinuxDisplay::renderPutPixel(int x, int y, uint16_t color) {
    int32_t r, g, b;

    if (m_format == Format::RGB565) {
        // rgb565 > rgba
        r = ((color & 0xF800) >> 11) << 3;
        g = ((color & 0x7E0) >> 5) << 2;
        b = ((color & 0x1F)) << 3;
    } else if (m_format == Format::ARGB444) {
        // argb4444 > rgba
        r = ((color & 0x0F00) >> 8) << 4;
        g = ((color & 0x00F0) >> 4) << 4;
        b = (color & 0x000F) << 4;
    }

    // draw the pixel to the renderer
    SDL_SetRenderDrawColor(p_renderer, r, g, b, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPoint(p_renderer, x, y);
}

void LinuxDisplay::flip() {
    // mimic pico display behavior
    auto surfaceSize = p_surfaces[0]->getSize();
    auto pixels = p_surfaces[0]->getPixels();
    auto pitch = getPitch();
    auto bpp = getBpp();
    auto maxWidth = std::min(surfaceSize.x, (int16_t) (m_displaySize.x - m_renderBounds.x));
    auto maxHeight = std::min(surfaceSize.y, (int16_t) (m_displaySize.y - m_renderBounds.y));
    if (maxWidth <= 0 || maxHeight <= 0) return;

    if (surfaceSize == m_displaySize) {
        // render size is equal to display size (fastest)
        for (int y = 0; y < m_displaySize.y; y++) {
            for (int x = 0; x < m_displaySize.x; x++) {
                renderPutPixel(x, y, *(uint16_t *) (p_surfaces[0]->getPixels() + y * m_pitch + x * m_bpp));
            }
        }

        SDL_RenderPresent(p_renderer);
        return;
    }

    if (surfaceSize == Utility::Vec2i(m_renderBounds.w, m_renderBounds.h)) {
        // surface size is equal to render size, draw without scaling  (fastest)
        for (int y = 0; y < maxHeight; y++) {
            for (int x = 0; x < maxWidth; x++) {
                renderPutPixel(x + m_renderBounds.x, y + m_renderBounds.y,
                               *(uint16_t *) (p_surfaces[0]->getPixels() + y * m_pitch + x * m_bpp));
            }
        }

        SDL_RenderPresent(p_renderer);
        return;
    }

    if (m_renderBounds.w / surfaceSize.x == 2 && m_renderBounds.h / surfaceSize.y == 2) {
        // scale2x
        for (int y = 0; y < maxHeight; y++) {
            for (int i = 0; i < 2; i++) {
                for (int x = 0; x < maxWidth; x += 2) {
                    // line 1
                    auto p1 = *(uint16_t *) (pixels + y * pitch + x * bpp);
                    renderPutPixel(x, y, p1);
                    renderPutPixel(x + 1, y, p1);
                    // line 2
                    auto p2 = *(uint16_t *) (pixels + y * pitch + (x + 1) * bpp);
                    renderPutPixel(x, y + 1, p2);
                    renderPutPixel(x + 1, y + 1, p2);
                }
            }
        }

        SDL_RenderPresent(p_renderer);
        return;
    }

    // nearest-neighbor scaling
    int x, y;
    int xRatio = (surfaceSize.x << 16) / m_renderBounds.w + 1;
    int yRatio = (surfaceSize.y << 16) / m_renderBounds.h + 1;
    for (int i = 0; i < m_renderBounds.h; i++) {
        for (int j = 0; j < m_renderBounds.w; j++) {
            x = (j * xRatio) >> 16;
            y = (i * yRatio) >> 16;
            renderPutPixel(x, y, *(uint16_t *) (pixels + y * pitch + x * bpp));
        }
    }

    SDL_RenderPresent(p_renderer);
}

LinuxDisplay::~LinuxDisplay() {
    printf("~LinuxDisplay()\n");
    SDL_DestroyRenderer(p_renderer);
    SDL_DestroyWindow(p_window);
    delete (p_surfaces[0]);
}
