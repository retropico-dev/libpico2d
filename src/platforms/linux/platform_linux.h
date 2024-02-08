//
// Created by cpasjuste on 30/05/23.
//

#ifndef MICROBOY_PLATFORM_LINUX_H
#define MICROBOY_PLATFORM_LINUX_H

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
    };
}

#endif //MICROBOY_PLATFORM_LINUX_H
