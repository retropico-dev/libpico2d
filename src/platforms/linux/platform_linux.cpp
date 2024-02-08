//
// Created by cpasjuste on 30/05/23.
//

#include "platform.h"

using namespace p2d;

LinuxPlatform::LinuxPlatform(const Display::Settings &settings) : Platform(settings) {
    Io::init();
    p_battery = new Battery();
    p_input = new LinuxInput();
    p_audio = new LinuxAudio();
    p_display = new LinuxDisplay(settings);
}

LinuxPlatform::~LinuxPlatform() {
    printf("~LinuxPlatform()\n");
    SDL_Quit();
}
