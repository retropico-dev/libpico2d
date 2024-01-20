//
// Created by cpasjuste on 19/01/24.
//

#ifndef PICO2D_SKELETON_SDCARD_H
#define PICO2D_SKELETON_SDCARD_H

namespace p2d {
    bool io_sdcard_init(float spiMhz = 20.0f); // TODO: bump speed on real hardware

    uint64_t io_sdcard_get_size();

    std::string io_sdcard_get_size_string();

    void io_sdcard_get_size(uint16_t &block_size, uint32_t &num_blocks);

    int32_t io_sdcard_read(uint32_t sector, uint32_t offset, void *buffer, uint32_t size_bytes);

    int32_t io_sdcard_write(uint32_t sector, uint32_t offset, const uint8_t *buffer, uint32_t size_bytes);
}

#endif //PICO2D_SKELETON_SDCARD_H
