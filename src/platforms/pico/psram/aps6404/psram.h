//
// Created by cpasjuste on 18/01/24.
//

#ifndef PICO2D_SKELETON_PSRAM_H
#define PICO2D_SKELETON_PSRAM_H

#ifdef PICO_PSRAM_RP2040

#include "aps6404.hpp"

#define PSRAM_INVALID_ADDRESS UINT32_MAX

namespace p2d {
    class PSram {
    public:
        static bool init();

        static void read(uint32_t addr, uint32_t *read_buf, const size_t len);

        static void read(uint32_t addr, uint16_t *read_buf, size_t len);

        static void read_blocking(uint32_t addr, uint32_t *read_buf, const size_t len);

        static void write(uint32_t addr, uint32_t *data, const size_t len);

        static void write(uint32_t addr, const uint16_t color, const size_t len);

        static void memset(uint32_t addr, uint32_t value, const size_t len);

        static void flush();

        static uint32_t alloc(size_t size, bool clear = false);

        // TODO
        //static void free(void *ptr);

        static pimoroni::APS6404 *getRam();
    };
}

#endif

#endif //PICO2D_SKELETON_PSRAM_H
