//
// Created by cpasjuste on 30/05/23.
//

#ifndef PICO2D_PICO_DISPLAY_H
#define PICO2D_PICO_DISPLAY_H

namespace mb {
    class PicoDisplay : public Display {
    public:
        explicit PicoDisplay(const Utility::Vec2i &displaySize = {240, 240},
                             const Utility::Vec2i &renderSize = {120, 120},
                             const ScaleMode &scaleMode = ScaleMode::Scale2x,
                             const Buffering &buffering = Buffering::Double);

        void setCursorPos(int16_t x, int16_t y) override;

        void setPixel(uint16_t color) override;

        void clear() override;

        void flip() override;

        Surface *getSurface(uint8_t index) {
            return p_surfaces[index];
        }

    private:
        Utility::Vec2i m_cursor;
        Surface *p_surfaces[2];
        uint8_t m_bufferIndex = 0;
    };
}

#endif //PICO2D_PICO_DISPLAY_H
