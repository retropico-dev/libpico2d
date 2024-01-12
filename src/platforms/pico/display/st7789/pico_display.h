//
// Created by cpasjuste on 30/05/23.
//

#ifndef PICO2D_PICO_DISPLAY_H
#define PICO2D_PICO_DISPLAY_H

namespace p2d {
    class PicoDisplay : public Display {
    public:
        explicit PicoDisplay(const Utility::Vec2i &displaySize = {240, 240},
                             const Utility::Vec2i &renderSize = {120, 120},
                             const Buffering &buffering = Buffering::Double,
                             const ScaleMode &scaleMode = ScaleMode::Scale2x);

        void setCursor(int16_t x, int16_t y) override;

        void setPixel(uint16_t color) override;

#ifndef PICO_DISPLAY_DIRECT_DRAW
        void clear() override;

        void flip() override;

        Surface *getSurface(uint8_t index) {
            return p_surfaces[index];
        }
#endif

    private:
        uint8_t m_bit_shift = 0;
#ifndef PICO_DISPLAY_DIRECT_DRAW
        Utility::Vec2i m_cursor;
        Surface *p_surfaces[2];
        uint8_t m_bufferIndex = 0;
#endif
    };
}

#endif //PICO2D_PICO_DISPLAY_H