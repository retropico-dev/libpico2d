//
// Created by cpasjuste on 30/05/23.
//

#ifndef MICROBOY_DISPLAY_LINUX_H
#define MICROBOY_DISPLAY_LINUX_H

// linux sdl2 display interface

namespace p2d {
    class LinuxDisplay : public Display {
    public:
        explicit LinuxDisplay(const Settings &settings)
                : LinuxDisplay(settings.displaySize, settings.renderSize,
                               settings.renderBounds, settings.bufferingMode,
                               settings.format, settings.spiSpeedMhz) {}

        explicit LinuxDisplay(const Utility::Vec2i &displaySize = {240, 240},
                              const Utility::Vec2i &renderSize = {240, 240},
                              const Utility::Vec4i &renderBounds = {0, 0, 240, 240},
                              const Buffering &buffering = Buffering::Double,
                              const Format &format = Format::RGB565,
                              float spiSpeedMhz = 62.5f);

        ~LinuxDisplay() override;

        void setCursor(int16_t x, int16_t y) override;

        void put(uint16_t color) override;

        void flip() override;

    private:
        SDL_Window *p_window = nullptr;
        SDL_Renderer *p_renderer = nullptr;
        Utility::Vec2i m_cursor{};

        void renderPutPixel(int x, int y, uint16_t color);
    };
}

#endif //MICROBOY_DISPLAY_LINUX_H
