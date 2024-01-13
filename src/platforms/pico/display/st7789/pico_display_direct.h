//
// Created by cpasjuste on 30/05/23.
//

#ifndef PICO2D_PICO_DISPLAY_DIRECT_H
#define PICO2D_PICO_DISPLAY_DIRECT_H

namespace p2d {
    class PicoDisplayDirectDraw : public Display {
    public:
        explicit PicoDisplayDirectDraw(const Settings &settings)
                : PicoDisplayDirectDraw(settings.displaySize, settings.renderSize,
                                        settings.bufferingMode, settings.scaleMode,
                                        settings.format, settings.spiSpeedMhz) {}

        explicit PicoDisplayDirectDraw(const Utility::Vec2i &displaySize = {240, 240},
                                       const Utility::Vec2i &renderSize = {120, 120},
                                       const Buffering &buffering = Buffering::Double,
                                       const ScaleMode &scaleMode = ScaleMode::Scale2x,
                                       const Format &format = Format::RGB565,
                                       float spiSpeedMhz = 62.5f);

        void setCursor(int16_t x, int16_t y) override;

        void setPixel(uint16_t color) override;

    private:
        uint8_t m_bit_shift = 0;

    };
}

#endif //PICO2D_PICO_DISPLAY_DIRECT_H