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

std::string Utility::parseSize(uint64_t size) {
    char output[32];
    const char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    char length = sizeof(suffix) / sizeof(suffix[0]);
    auto dblBytes = (double) size;
    int i = 0;

    if (size <= 0) {
        return "0 B";
    } else {
        if (size > 1024) {
            for (i = 0; (size / 1024) > 0 && i < length - 1; i++, size /= 1024)
                dblBytes = (double) size / 1024.0;
        }

        snprintf(output, 31, "~%i %s", (int) dblBytes, suffix[i]);
        return output;
    }
}

void Utility::timerStart() {
    m_timing_clock.restart();
}

uint64_t Utility::timerStopPrintMicro() {
    uint64_t t = m_timing_clock.getElapsedTime().asMicroseconds();
    printf("%llu\r\n", t);
    return t;
}

uint32_t Utility::timerStopPrintMillis() {
    uint32_t t = m_timing_clock.getElapsedTime().asMilliseconds();
    printf("%lu\r\n", t);
    return t;
}
