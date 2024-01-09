//
// Created by cpasjuste on 09/01/24.
//

#ifndef PICO2D_SKELETON_ROMFS_H
#define PICO2D_SKELETON_ROMFS_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace p2d {
    class RomFs {
    public:
        struct Binary {
            const uint8_t *data = nullptr;
            const size_t size = 0;
        };

        static Binary get(const std::string &path);

        static bool exists(const std::string &path);
    };
}

#endif //PICO2D_SKELETON_ROMFS_H
