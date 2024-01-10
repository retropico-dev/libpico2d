//
// Created by cpasjuste on 30/05/23.
//

#include <cstdio>
#include "platform.h"
#include "display_linux.h"

using namespace p2d;

LinuxDisplay::LinuxDisplay(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                           const Buffering &buffering, const ScaleMode &scaleMode)
        : Display(displaySize, renderSize, buffering, scaleMode) {
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

    if (displaySize != renderSize) {
        SDL_RenderSetLogicalSize(p_renderer, renderSize.x, renderSize.y);
    }

    printf("LinuxDisplay: %ix%i (texture pitch: %i)\n", m_displaySize.x, m_displaySize.y, m_pitch);
}

void LinuxDisplay::setCursorPos(int16_t x, int16_t y) {
    m_cursor = {x, y};
}

void LinuxDisplay::setPixel(uint16_t color) {
    if (color != Display::Color::Transparent) {
        // rgb565 > rgb32
        int32_t r = ((color & 0xF800) >> 11) << 3;
        int32_t g = ((color & 0x7E0) >> 5) << 2;
        int32_t b = ((color & 0x1F)) << 3;

        // draw the pixel to the renderer
        SDL_SetRenderDrawColor(p_renderer, r, g, b, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawPoint(p_renderer, m_cursor.x, m_cursor.y);
    }

    // emulate tft lcd "put_pixel" buffer
    m_cursor.x++;
    if (m_cursor.x >= m_displaySize.x) {
        m_cursor.x = 0;
        m_cursor.y += 1;
    }
}

void LinuxDisplay::flip() {
    SDL_RenderPresent(p_renderer);
}

LinuxDisplay::~LinuxDisplay() {
    printf("~LinuxDisplay()\n");
    SDL_DestroyRenderer(p_renderer);
    SDL_DestroyWindow(p_window);
}
