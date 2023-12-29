//
// Created by cpasjuste on 30/05/23.
//

#ifndef MICROBOY_DISPLAY_H
#define MICROBOY_DISPLAY_H

#include "Adafruit_GFX.h"
#include "surface.h"
#include "utility.h"

namespace mb {
    class Display : public Adafruit_GFX {
    public:
        enum Format {
            RGB444,
            RGB555,
            RGB565
        };

        enum Color {
            Black = 0x0000,
            Blue = 0x001F,
            Red = 0xF800,
            Green = 0x07E0,
            Cyan = 0x07FF,
            Magenta = 0xF81F,
            Yellow = 0xFFE0,
            White = 0xFFFF
        };

        // init a display (hardware dependant, to be implemented)
        // default display size used for "ST7789 1.54" TFT IPS 240x240"
        explicit Display(const Utility::Vec2i &size = {240, 240});

        // destroy the display (hardware dependant, to be implemented)
        ~Display() override;

        // set the position inside pixel buffer (hardware dependant, to be implemented)
        virtual void setCursor(uint16_t x, uint16_t y) {};

        // set pixel color at cursor position
        virtual void setPixel(uint16_t color) {};

        // flip the display (hardware dependant, to be implemented)
        virtual void flip() {}

        // clear the display
        virtual void clear(uint16_t color = Black);

        // draw a pixel to the display (slow)
        void drawPixel(int16_t x, int16_t y, uint16_t color) override;

        // draw a pixel to the display (slow)
        void drawPixel(const Utility::Vec2i &pos, uint16_t color) {
            drawPixel(pos.x, pos.y, color);
        }

        // draw a pixel line buffer to the display (fast)
        void drawPixelLine(const uint16_t *pixels, uint16_t width, const Format &format = RGB565);

        // draw a surface (pixel buffer) to the display with scaling if requested
        void drawSurface(Surface *surface, const Utility::Vec2i &pos, const Utility::Vec2i &size);

        // draw a surface (pixel buffer) to the display
        void drawSurface(Surface *surface, const Utility::Vec2i &pos) {
            if (!surface) return;
            drawSurface(surface, pos, surface->getSize());
        }

        // draw a surface (pixel buffer) to the display
        void drawSurface(Surface *surface) {
            if (!surface) return;
            drawSurface(surface, {0, 0}, surface->getSize());
        }

        void drawText(int16_t x, int16_t y, const std::string &text) {
            Adafruit_GFX::setCursor(x, y);
            print(text.c_str());
        }

        void drawText(const Utility::Vec2i &pos, const std::string &text) {
            drawText(pos.x, pos.y, text);
        }

        void setClipArea(const Utility::Vec4i &clipArea) {
            m_clip = clipArea;
        }

        [[nodiscard]] Utility::Vec4i getClipArea() const {
            return m_clip;
        }

        // get display size
        Utility::Vec2i getSize() { return m_size; };

    protected:
        uint16_t *m_line_buffer;
        Utility::Vec2i m_size{};
        Utility::Vec4i m_clip{};
        int m_pitch = 0;
        int m_bpp = 2;
    };
}

#endif //MICROBOY_DISPLAY_H
