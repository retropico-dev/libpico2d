//
// Created by cpasjuste on 30/05/23.
//

#ifndef P2D_PLATFORM_LINUX_H
#define P2D_PLATFORM_LINUX_H

#include <SDL2/SDL.h>
#include "display_linux.h"
#include "input_linux.h"
#include "audio_linux.h"
#include "pico.h"

namespace p2d {
    class LinuxPlatform : public Platform {
    public:
        explicit LinuxPlatform(const Display::Settings &displaySettings);

        ~LinuxPlatform() override;

        void sleep(uint32_t ms) override;
    };
}

#endif //P2D_PLATFORM_LINUX_H
