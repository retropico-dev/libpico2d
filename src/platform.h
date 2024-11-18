//
// Created by cpasjuste on 30/05/23.
//

#ifndef PICO2D_PLATFORM_H
#define PICO2D_PLATFORM_H

#include <cstdio>
#include "utility"
#include "display.h"
#include "input.h"
#include "audio.h"
#include "io.h"
#include "clock.h"
#include "widget.h"
#include "battery.h"

#ifdef PICO_PSRAM_RP2040

#include "psram.h"

#endif

namespace p2d {
    class Platform : public Widget {
    public:
        explicit Platform(const Display::Settings &displaySettings);

        virtual ~Platform();

        virtual bool loop();

        virtual void reboot(uint32_t watchdog_scratch = 0) {
        }

        virtual void sleep(uint32_t ms) {
        }

        void onUpdate(Time delta) override;

        bool onInput(const uint16_t &buttons) override;

        void onDraw(bool draw = true) override;

        void add(Widget *widget) override { Widget::add(widget); }

        void remove(Widget *widget) override { Widget::remove(widget); };

        Display *getDisplay() { return p_display; }

        Audio *getAudio() { return p_audio; }

        Input *getInput() { return p_input; }

        Battery *getBattery() { return p_battery; }

        static Platform *instance();

    protected:
        Display *p_display = nullptr;
        Input *p_input = nullptr;
        Audio *p_audio = nullptr;
        Battery *p_battery = nullptr;
        Clock m_elapsed_clock{}, m_delta_clock{}, m_fps_clock{}, m_special_buttons_clock{};
        uint16_t m_fps = 0, m_frames = 0;
        uint32_t m_runtime_minutes = 0;
        bool m_stats_print = true;
    };
}

#ifdef LINUX
#include "platform_linux.h"
#define P2DPlatform p2d::LinuxPlatform
#define P2DDisplay p2d::LinuxDisplay
#define IN_RAM
#define IN_FLASH
#define IN_PSRAM
#else
#include "platform_pico.h"
#define P2DPlatform p2d::PicoPlatform
#define P2DDisplay p2d::PicoDisplay
#define IN_RAM __attribute__((section(".time_critical.")))
#define IN_FLASH __attribute__((section(".flashdata.")))
#ifdef GPIO_PIN_PSRAM_CS
#define IN_PSRAM __attribute__((section(".psramdata.")))
#else
#define IN_PSRAM
#endif
#endif

#endif //PICO2D_PLATFORM_H
