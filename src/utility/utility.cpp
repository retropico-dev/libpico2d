//
// Created by cpasjuste on 30/05/23.
//

#include <algorithm>
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

std::string Utility::toLower(const std::string &str) {
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return ret;
}

std::string Utility::toUpper(const std::string &str) {
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return ret;
}

bool Utility::startWith(const std::string &str, const std::string &value, bool sensitive) {
    if (sensitive) {
        return str.compare(0, value.length(), value) == 0;
    } else {
        std::string str_low = toLower(str);
        std::string val_low = toLower(value);
        return str_low.compare(0, val_low.length(), val_low) == 0;
    }
}

bool Utility::endsWith(const std::string &value, const std::string &ending, bool sensitive) {
    if (ending.size() > value.size()) return false;
    if (sensitive) {
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    } else {
        std::string val_low = toLower(value);
        std::string end_low = toLower(ending);
        return std::equal(end_low.rbegin(), end_low.rend(), val_low.rbegin());
    }
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
