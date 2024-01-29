//
// Created by cpasjuste on 29/01/24.
//

#include "cmrc/cmrc.hpp"
#include "io.h"

using namespace cmrc;
CMRC_DECLARE(pico2d);

static void load_resources(const embedded_filesystem &fs, const std::string &path = "") {
    for (auto &&entry: fs.iterate_directory(path)) {
        std::string p = path + "/" + entry.filename();
        if (entry.is_directory()) {
            load_resources(fs, p);
        } else {
            auto file = fs.open(p);
            auto resPath = "res:" + p;
            if (resPath == "res:/src/romfs/.keep") continue;
            p2d::Io::File::addBufferFile(resPath, (const uint8_t *) file.cbegin(), file.size());
            printf("Io: resource added: \"%s\"\r\n", resPath.c_str());
        }
    }
}

void p2d::load_romfs() {
    auto fs = pico2d::get_filesystem();
    load_resources(fs);
}
