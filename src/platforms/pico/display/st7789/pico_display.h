//
// Created by cpasjuste on 30/05/23.
//

#ifndef PICO2D_PICO_DISPLAY_H
#define PICO2D_PICO_DISPLAY_H

namespace p2d {
    class PicoDisplay : public Display {
    public:
        explicit PicoDisplay(const Settings &settings)
                : PicoDisplay(settings.displaySize, settings.renderSize,
                              settings.renderBounds, settings.bufferingMode,
                              settings.format, settings.spiSpeedMhz) {}

        explicit PicoDisplay(const Utility::Vec2i &displaySize = {240, 240},
                             const Utility::Vec2i &renderSize = {240, 240},
                             const Utility::Vec4i &renderBounds = {0, 0, 240, 240},
                             const Buffering &buffering = Buffering::Double,
                             const Format &format = Format::RGB565,
                             float spiSpeedMhz = 62.5f);

        void setCursor(int16_t x, int16_t y) override;

        void setPixel(uint16_t color) override;

        void clear() override;

        void flip() override;

    private:
        void clearDisplay();

        uint8_t m_bit_shift = 0;
        Utility::Vec2i m_cursor;
    };
}

#endif //PICO2D_PICO_DISPLAY_H