//
// Created by cpasjuste on 29/12/23.
//

#include "platform.h"

using namespace mb;

static Platform *s_platform = nullptr;

Platform::Platform(bool overclock) {
    s_platform = this;
}

Platform *Platform::instance() {
    return s_platform;
}
