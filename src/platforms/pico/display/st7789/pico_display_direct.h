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
                                        settings.renderBounds, settings.bufferingMode,
                                        settings.format, settings.spiSpeedMhz) {}

        explicit PicoDisplayDirectDraw(const Utility::Vec2i &displaySize = {240, 240},
                                       const Utility::Vec2i &renderSize = {240, 240},
                                       const Utility::Vec4i &renderBounds = {0, 0, 240, 240},
                                       const Buffering &buffering = Buffering::Double,
                                       const Format &format = Format::RGB565,
                                       float spiSpeedMhz = 70.0f);

        void setDisplayBounds(int16_t x, int16_t y, uint16_t w, uint16_t h) override;

        void setCursor(int16_t x, int16_t y) override;

        void put(uint16_t color) override;

        void put(const uint16_t *buffer, uint32_t count) override;

        void putFast(const uint16_t *buffer, uint32_t count) override;

        void clear() override;

        void flush() override;

    private:
        uint8_t m_bit_shift = 0;

    };
}

#endif //PICO2D_PICO_DISPLAY_DIRECT_H