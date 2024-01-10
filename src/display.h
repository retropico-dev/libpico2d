//
// Created by cpasjuste on 30/05/23.
//

#ifndef MICROBOY_DISPLAY_H
#define MICROBOY_DISPLAY_H

#include "Adafruit_GFX.h"
#include "surface.h"
#include "utility.h"

namespace p2d {
    class Display : public Adafruit_GFX {
    public:
        enum Buffering {
            Single, // use a single buffer (screen (w * h * bpp) mem usage)
            Double  // use two buffers + core1 rendering (screen (w * h * bpp * 2) mem usage)
        };

        enum Format {
            RGB444,
            //RGB555,
            RGB565
        };

        enum ScaleMode {
            Scale2x,    // integer scaling, fast
            Scanline2x, // integer scaling, fast
            Nearest     // free scaling ratio, slower
        };

        enum Color {
            Black = 0x0000,
            White = 0xFFFF,
            Red = 0xC083,
            Yellow = 0xFF00,
            YellowLight = 0xF5C1,
            Green = 0x07E0,
            GreenDark = 0x0388,
            Blue = 0x01CF,
            Gray = 0x52CB,
            GrayDark = 0x2986,
            Transparent = 0x0588 // #00b242
        };

        // init a display (hardware dependant, to be implemented)
        // default display size used for "ST7789 1.54" TFT IPS 240x240"
        explicit Display(const Utility::Vec2i &displaySize = {240, 240},
                         const Utility::Vec2i &renderSize = {240, 240},
                         const Buffering &buffering = Buffering::Double,
                         const ScaleMode &scaleMode = ScaleMode::Scale2x);

        // destroy the display (hardware dependant, to be implemented)
        virtual ~Display();

        // set the position inside pixel buffer (hardware dependant, to be implemented)
        virtual void setCursorPos(int16_t x, int16_t y) {};

        // set pixel color at cursor position
        virtual void setPixel(uint16_t color) {};

        // flip the display (hardware dependant, to be implemented)
        virtual void flip() {}

        // clear the display
        virtual void clear();

        // draw a pixel to the display (slow)
        void drawPixel(int16_t x, int16_t y, uint16_t color) override;

        // draw a pixel to the display (slow)
        void drawPixel(const Utility::Vec2i &pos, uint16_t color) {
            drawPixel(pos.x, pos.y, color);
        }

        // draw a pixel line buffer to the display (fast)
        virtual void drawPixelLine(const uint16_t *pixels, uint16_t width, const Format &format = RGB565);

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
        Utility::Vec2i getDisplaySize() { return m_displaySize; };

        Utility::Vec2i getSize() { return m_renderSize; };

        Utility::Vec4i getBounds() {
            return {0, 0, m_renderSize.x, m_renderSize.y};
        }

        int getPitch() { return m_pitch; };

        int getBpp() { return m_bpp; };

        ScaleMode getScaleMode() { return m_scaleMode; }

        void setScaleMode(const ScaleMode &scaleMode) {
            m_scaleMode = scaleMode;
        }

        [[nodiscard]] uint16_t getClearColor() const { return m_clearColor; }

        void setClearColor(uint16_t color) { m_clearColor = color; }

    protected:
        uint16_t m_clearColor = Color::Black;
        uint16_t m_colorKey = Color::Transparent;
        uint16_t *m_line_buffer;
        Utility::Vec4i m_clip{};
        Buffering m_buffering = Buffering::Double;
        Utility::Vec2i m_displaySize{};
        Utility::Vec2i m_renderSize{};
        ScaleMode m_scaleMode = ScaleMode::Scale2x;
        int m_pitch = 0;
        int m_bpp = 2;
    };
}

#endif //MICROBOY_DISPLAY_H
