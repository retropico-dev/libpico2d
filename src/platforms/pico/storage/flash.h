//
// Created by cpasjuste on 19/01/24.
//

#ifndef PICO2D_SKELETON_FLASH_H
#define PICO2D_SKELETON_FLASH_H

#include <cstdio>
#include <string>
#include <hardware/flash.h>

// 16MB flash: 5MB for bootloader/emulators/cache, 10MB for fatfs "user" data/cache
#define FLASH_TARGET_OFFSET_FATFS ((1024 * 1024) * 5)

namespace p2d {
    bool io_flash_init();

    uint64_t io_flash_get_size();

    void io_flash_get_size(uint16_t &block_size, uint32_t &num_blocks);

    std::string io_flash_get_size_string();

    int32_t io_flash_read(uint32_t sector, uint32_t offset, void *buffer, uint32_t size_bytes);

    int32_t io_flash_write(uint32_t sector, uint32_t offset, const uint8_t *buffer, uint32_t size_bytes);
}

#endif //PICO2D_SKELETON_FLASH_H
