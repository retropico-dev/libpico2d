//
// Created by cpasjuste on 30/05/23.
//

#ifndef P2D_DISPLAY_H
#define P2D_DISPLAY_H

#ifndef ARDUINO
#define ARDUINO 123
#endif

#include "Adafruit_GFX.h"
#include "surface.h"
#include "utility.h"

namespace p2d {
    // TODO: make Adafruit_GFX private
    class Display : public Adafruit_GFX {
    public:
        enum class Buffering {
            None,
            Single, // use a single buffer (screen (w * h * bpp) mem usage)
            Double  // use two buffers + core1 rendering (screen (w * h * bpp * 2) mem usage)
        };

        enum class Format {
            ARGB444, // infones
            RGB444,
            RGB565
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

        struct Settings {
            Utility::Vec2i displaySize = {240, 240};
            Utility::Vec2i renderSize = {240, 240};
            Utility::Vec4i renderBounds = {0, 0, 240, 240};
            Buffering bufferingMode = Buffering::Double;
            Format format = Format::RGB565;
            float spiSpeedMhz = 70.0f;
        };

        explicit Display(const Settings &settings)
                : Display(settings.displaySize, settings.renderSize,
                          settings.renderBounds, settings.bufferingMode,
                          settings.format, settings.spiSpeedMhz) {}

        // init a display (hardware dependant, to be implemented)
        // default display size used for "ST7789 1.54" TFT IPS 240x240"
        explicit Display(const Utility::Vec2i &displaySize = {240, 240},        // real display size
                         const Utility::Vec2i &renderSize = {240, 240},         // render size
                         const Utility::Vec4i &renderBounds = {0, 0, 240, 240}, // scale if renderBounds > renderSize
                         const Buffering &buffering = Buffering::Double,
                         const Format &format = Format::RGB565,
                         float spiSpeedMhz = 70.0f);

        // destroy the display (hardware dependant, to be implemented)
        ~Display() override;

        virtual void setDisplayBounds(int16_t x, int16_t y, uint16_t w, uint16_t h) {};

        // set the position inside pixel buffer (hardware dependant, to be implemented)
        virtual void setCursor(int16_t x, int16_t y) {};

        // flip the display (hardware dependant, to be implemented)
        virtual void flip() {}

        // clear the display
        virtual void clear();

        // flush display
        virtual void flush() {}

        // put a pixel at current cursor position
        virtual void put(uint16_t color) {};

        // put a pixel to the specific position (slow)
        void put(int16_t x, int16_t y, uint16_t color) {
            setCursor(x, y);
            put(color);
        }

        // put a pixel to the specific position (slow)
        void put(const Utility::Vec2i &pos, uint16_t color) {
            put(pos.x, pos.y, color);
        }

        // put x pixels from current cursor position
        virtual void put(const uint16_t *buffer, uint32_t count);

        // put x count of pixels from current cursor position, without alpha color key handling
        virtual void putFast(const uint16_t *buffer, uint32_t count) {
            for (int i = 0; i < count; i++) put(buffer[i]);
        }

        // draw a surface (pixel buffer) to the display with scaling if requested
        virtual void drawSurface(Surface *surface, const Utility::Vec4i &bounds);

        // draw a surface (pixel buffer) to the display
        void drawSurface(Surface *surface, const Utility::Vec2i &pos) {
            if (!surface) return;
            drawSurface(surface, {pos.x, pos.y, surface->getSize().x, surface->getSize().y});
        }

        // draw a surface (pixel buffer) to the display
        void drawSurface(Surface *surface) {
            if (!surface) return;
            drawSurface(surface, {0, 0, surface->getSize().x, surface->getSize().y});
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

        [[nodiscard]] Utility::Vec2i getSize() const { return m_renderSize; }

        [[nodiscard]] int getPitch() const { return m_pitch; };

        [[nodiscard]] int getBpp() const { return m_bpp; };

        [[nodiscard]] Color getClearColor() const { return m_clearColor; }

        void setClearColor(Color color) { m_clearColor = color; }

        [[nodiscard]]  Format getFormat() const { return m_format; }

        void setFormat(const Format &fmt) {
//#warning "TODO: implement Display::setFormat"
            m_format = fmt;
        }

        Surface *getFramebuffer() {
            return p_surfaces[m_bufferIndex];
        }

    protected:
        int m_bpp = 2;
        int m_pitch = 0;
        Format m_format = Format::RGB565;
        Color m_clearColor = Color::Black;
        Color m_colorKey = Color::Transparent;
        uint16_t *m_line_buffer;
        Utility::Vec4i m_clip{};
        Utility::Vec2i m_displaySize{};     // real display size (hardware)
        Utility::Vec2i m_renderSize{};      // rendering size (framebuffer size)
        Utility::Vec4i m_renderBounds{};    // rendering bounds, used for scaling

        // frame-buffers
        Buffering m_buffering = Buffering::Double;
        uint8_t m_bufferIndex = 0;
        Surface *p_surfaces[2];

        Surface *getSurface(uint8_t index) {
            return p_surfaces[index];
        }

        void drawPixel(int16_t x, int16_t y, uint16_t color) override {
            put(x, y, color);
        }
    };
}

#endif //P2D_DISPLAY_H
