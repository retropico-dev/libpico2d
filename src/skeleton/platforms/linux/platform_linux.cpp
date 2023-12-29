//
// Created by cpasjuste on 30/05/23.
//

#include "platform.h"

using namespace mb;

LinuxPlatform::LinuxPlatform(bool useDoubleBufferDisplay, bool overclock) : Platform() {
    p_display = (Display *) new LinuxDisplay();
    p_audio = new LinuxAudio();
    p_input = new LinuxInput();
    p_io = new LinuxIo();

    // set rendering size to display size
    Rectangle::setSize(p_display->getSize());
}

LinuxPlatform::~LinuxPlatform() {
    printf("~LinuxPlatform()\n");
    SDL_Quit();
}
