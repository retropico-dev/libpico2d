//
// Created by cpasjuste on 30/05/23.
//

#include "platform.h"

using namespace mb;

LinuxPlatform::LinuxPlatform(const Display::Buffering &buffering, bool overclock)
        : Platform(buffering, overclock) {
    p_display = (Display *) new LinuxDisplay({240, 240}, {120, 120});
    p_audio = new LinuxAudio();
    p_input = new LinuxInput();
    p_io = new LinuxIo();

    // set rendering size to display size
    Rectangle::setSize(p_display->getRenderSize());
}

LinuxPlatform::~LinuxPlatform() {
    printf("~LinuxPlatform()\n");
    SDL_Quit();
}
