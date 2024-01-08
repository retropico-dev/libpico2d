//
// Created by cpasjuste on 30/05/23.
//

#include "platform.h"

using namespace p2d;

LinuxPlatform::LinuxPlatform(bool overclock) : Platform(overclock) {
    p_audio = new LinuxAudio();
    p_input = new LinuxInput();
    p_io = new LinuxIo();
}

LinuxPlatform::~LinuxPlatform() {
    printf("~LinuxPlatform()\n");
    SDL_Quit();
}
