//
// Created by cpasjuste on 30/05/23.
//

#include "platform.h"

using namespace p2d;

LinuxPlatform::LinuxPlatform() : Platform() {
    Io::init();
    p_audio = new LinuxAudio();
    p_input = new LinuxInput();
}

LinuxPlatform::~LinuxPlatform() {
    printf("~LinuxPlatform()\n");
    SDL_Quit();
}
