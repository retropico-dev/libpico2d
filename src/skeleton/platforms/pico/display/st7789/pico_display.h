//
// Created by cpasjuste on 30/05/23.
//

#ifndef MICROBOY_PICO_DISPLAY_H
#define MICROBOY_PICO_DISPLAY_H

namespace mb {
    class PicoDisplay : public Display {
    public:
        PicoDisplay();

        void setCursorPos(int16_t x, int16_t y) override;

        void setPixel(uint16_t color) override;

    private:
        Utility::Vec2i m_cursor;
    };
}

#endif //MICROBOY_PICO_DISPLAY_H
