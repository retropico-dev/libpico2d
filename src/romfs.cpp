//
// Created by cpasjuste on 09/01/24.
//

#include "romfs.h"
#include <cmrc/cmrc.hpp>

using namespace p2d;
using namespace cmrc;
CMRC_DECLARE(pico2d);

bool RomFs::exists(const std::string &path) {
    auto fs = pico2d::get_filesystem();
    return fs.exists(path);
}

RomFs::Binary RomFs::get(const std::string &path) {
    auto fs = pico2d::get_filesystem();
    if (fs.exists(path)) {
        auto file = fs.open(path);
        return {(const uint8_t *) file.cbegin(), file.size()};
    }

    return {};
}
