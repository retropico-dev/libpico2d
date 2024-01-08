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
#include "rectangle.h"
#include "resource.h"

namespace mb {
    class Platform : public Rectangle {
    public:
        explicit Platform(bool overclock = false);

        virtual ~Platform() {
            printf("~Platform()\n");
            delete (p_display);
            delete (p_audio);
            delete (p_input);
            delete (p_io);
        };

        static Platform *instance();

        void addDisplay(Display *display) {
            p_display = display;
            Rectangle::setSize(p_display->getRenderSize());
        }

        Display *getDisplay() { return p_display; }

        Audio *getAudio() { return p_audio; }

        Input *getInput() { return p_input; }

        Io *getIo() { return p_io; }

        virtual bool loop(bool forceDraw = false);

        virtual void reboot() {};

    protected:
        Display *p_display = nullptr;
        Input *p_input = nullptr;
        Audio *p_audio = nullptr;
        Io *p_io = nullptr;
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