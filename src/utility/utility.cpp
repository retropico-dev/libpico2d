//
// Created by cpasjuste on 30/05/23.
//

#include "utility.h"

using namespace p2d;

static Clock m_timing_clock;

std::string Utility::removeExt(const std::string &str) {
    size_t pos = str.find_last_of('.');
    if (pos != std::string::npos) {
        return str.substr(0, pos);
    } else {
        return str;
    }
}

std::string Utility::baseName(const std::string &path) {
    std::string name = path;
    if (path.size() > 1) {
        const size_t idx = path.find_last_of('/');
        if (idx != std::string::npos) {
            name.erase(0, idx + 1);
        }
    }
    return name;
}

void Utility::timerStart() {
    m_timing_clock.restart();
}

uint64_t Utility::timerStopPrintMicro() {
    uint64_t t = m_timing_clock.getElapsedTime().asMicroseconds();
    printf("%llu\r\n", t);
    return t;
}
