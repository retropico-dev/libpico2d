//
// Created by cpasjuste on 19/01/24.
//

// source: https://github.com/32blit/32blit-sdk/blob/master/32blit-pico/storage_sd_spi.cpp

#include <cstdio>
#include <csignal>
#include <sys/stat.h>

#include "utility.h"
#include "sdcard.h"

#define SD_PATH "sd.img"
#define PICO_SDCARD_SIZE_BYTES (128 * 1024 * 1024)
#define BLOCK_SIZE 512
#define BLOCK_COUNT (PICO_SDCARD_SIZE_BYTES / BLOCK_SIZE)

static FILE *fh;
static bool sd_io_initialised = false;

bool p2d::io_sdcard_init(float spiMhz) {
    if (sd_io_initialised) return true;

    struct stat st{};
    if (stat(SD_PATH, &st) != 0) {
        fh = fopen(SD_PATH, "w+");
        ftruncate(fileno(fh), PICO_SDCARD_SIZE_BYTES);
        fclose(fh);
    }

    fh = fopen(SD_PATH, "r+");
    sd_io_initialised = fh != nullptr;

    return sd_io_initialised;
}

void p2d::io_sdcard_get_size(uint16_t &block_size, uint32_t &num_blocks) {
    block_size = BLOCK_SIZE;
    num_blocks = BLOCK_COUNT;
}

uint64_t p2d::io_sdcard_get_size() {
    return PICO_SDCARD_SIZE_BYTES;
}

std::string p2d::io_sdcard_get_size_string() {
    return p2d::Utility::parseSize(PICO_SDCARD_SIZE_BYTES);
}

int32_t p2d::io_sdcard_read(uint32_t sector, uint32_t offset, void *buffer, uint32_t size_bytes) {
    fseek(fh, sector * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, size_bytes / BLOCK_SIZE, fh);

    return (int32_t) size_bytes;
}

int32_t p2d::io_sdcard_write(uint32_t sector, uint32_t offset, const uint8_t *buffer, uint32_t size_bytes) {
    fseek(fh, sector * BLOCK_SIZE, SEEK_SET);
    size_t written = fwrite(buffer, BLOCK_SIZE, size_bytes / BLOCK_SIZE, fh) * BLOCK_SIZE;

    return (int32_t) written;
}

void p2d::io_sdcard_exit() {
    if (fh) fclose(fh);
}
