//
// Created by cpasjuste on 30/05/23.
//

#ifndef MICROBOY_DISPLAY_LINUX_H
#define MICROBOY_DISPLAY_LINUX_H

// linux sdl2 display interface

namespace p2d {
    class LinuxDisplay : public Display {
    public:
        LinuxDisplay(const Utility::Vec2i &displaySize = {240, 240},
                     const Utility::Vec2i &renderSize = {240, 240},
                     const ScaleMode &scaleMode = ScaleMode::Scale2x,
                     const Buffering &buffering = Buffering::Double);

        ~LinuxDisplay() override;

        void setCursorPos(int16_t x, int16_t y) override;

        void setPixel(uint16_t color) override;

        void flip() override;

    private:
        SDL_Window *p_window = nullptr;
        SDL_Renderer *p_renderer = nullptr;
        Utility::Vec2i m_cursor{};
    };
}

#endif //MICROBOY_DISPLAY_LINUX_H
