//
// Created by cpasjuste on 30/05/23.
//

#ifndef PICO2D_PLATFORM_H
#define PICO2D_PLATFORM_H

#include <cstdio>
#include "display.h"
#include "input.h"
#include "audio.h"
#include "io.h"
#include "clock.h"
#include "utility"
#include "widget.h"

#ifdef PICO_PSRAM

#include "psram.h"

#endif

namespace p2d {
    class Platform : Widget {
    public:
        explicit Platform(bool overclock = false);

        virtual ~Platform() {
            printf("~Platform()\n");
            delete (p_display);
            delete (p_audio);
            delete (p_input);
        };

        static Platform *instance();

        void setDisplay(Display *display) {
            p_display = display;
            Widget::setPosition(0, 0);
            Widget::setSize(p_display->getSize());
        }

        Display *getDisplay() { return p_display; }

        Io *getIo() { return p_io; }

        Audio *getAudio() { return p_audio; }

        Input *getInput() { return p_input; }

        virtual bool loop(bool forceDraw = false);

        virtual void reboot(uint32_t watchdog_scratch = 0) {};

        void add(Widget *widget) override { Widget::add(widget); }

        void remove(Widget *widget) override { Widget::remove(widget); };

    protected:
        Display *p_display = nullptr;
        Io *p_io = nullptr;
        Input *p_input = nullptr;
        Audio *p_audio = nullptr;
    };
}

#ifdef LINUX
#include "platform_linux.h"
#define P2DPlatform LinuxPlatform
#define P2DDisplay LinuxDisplay
#else

#include "platform_pico.h"

#define P2DPlatform PicoPlatform
#define P2DDisplay PicoDisplay
#endif

#endif //PICO2D_PLATFORM_H
