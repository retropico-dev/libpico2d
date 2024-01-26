//
// Created by cpasjuste on 18/01/24.
//

#ifndef PICO2D_SKELETON_PSRAM_H
#define PICO2D_SKELETON_PSRAM_H

#ifdef PICO_PSRAM

#define PSRAM_INVALID_ADDRESS UINT32_MAX

namespace p2d {
    class PSram {
    public:
        static bool init();

        static void read(uint32_t addr, uint32_t *read_buf, size_t count);

        static void write(uint32_t addr, uint32_t *data, size_t count);

        static void memset(uint32_t addr, uint32_t value, size_t count);

        static uint32_t alloc(size_t size);

        // TODO
        //static void free(void *ptr);
    };
}

#endif

#endif //PICO2D_SKELETON_PSRAM_H
