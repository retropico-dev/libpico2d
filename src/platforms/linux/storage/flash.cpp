//
// Created by cpasjuste on 19/01/24.
//

// source: https://github.com/32blit/32blit-sdk/blob/master/32blit-pico/storage_sd_spi.cpp

#include <cstdio>
#include <sys/stat.h>
#include <csignal>
#include "utility.h"
#include "io.h"

#define FLASH_PATH "flash.raw"
#define FLASH_SIZE_BYTES_FULL (16 * 1024 * 1024)
#define FLASH_SIZE_BYTES_FATFS (FLASH_SIZE_BYTES_FULL - FLASH_TARGET_OFFSET_FATFS)
#define BLOCK_SIZE 4096
#define BLOCK_COUNT (FLASH_SIZE_BYTES_FATFS / BLOCK_SIZE)

static FILE *fh;
static bool flash_io_initialised = false;

bool p2d::io_flash_init() {
    if (flash_io_initialised) return true;

    struct stat st{};
    if (stat(FLASH_PATH, &st) != 0) {
        fh = fopen(FLASH_PATH, "w+");
        ftruncate(fileno(fh), FLASH_SIZE_BYTES_FULL);
        fclose(fh);
    }

    fh = fopen(FLASH_PATH, "r+");
    flash_io_initialised = fh != nullptr;

    return flash_io_initialised;
}

void p2d::io_flash_get_size(uint16_t &block_size, uint32_t &num_blocks) {
    block_size = BLOCK_SIZE;
    num_blocks = BLOCK_COUNT;
}

uint64_t p2d::io_flash_get_size() {
    return (uint64_t) FLASH_SIZE_BYTES_FATFS;
}

std::string p2d::io_flash_get_size_string() {
    return p2d::Utility::parseSize(FLASH_SIZE_BYTES_FATFS);
}

int32_t p2d::io_flash_read(uint32_t sector, uint32_t offset, void *buffer, uint32_t size_bytes) {
    fseek(fh, FLASH_TARGET_OFFSET_FATFS + sector * BLOCK_SIZE, SEEK_SET);
    fread(buffer, BLOCK_SIZE, size_bytes / BLOCK_SIZE, fh);

    return (int32_t) size_bytes;
}

int32_t p2d::io_flash_write(uint32_t sector, uint32_t offset, const uint8_t *buffer, uint32_t size_bytes) {
    fseek(fh, FLASH_TARGET_OFFSET_FATFS + sector * BLOCK_SIZE, SEEK_SET);
    size_t written = fwrite(buffer, BLOCK_SIZE, size_bytes / BLOCK_SIZE, fh) * BLOCK_SIZE;

    return (int32_t) written;
}

int32_t p2d::io_flash_read_sector(uint32_t offset, void *buffer) {
    fseek(fh, offset, SEEK_SET);
    fread(buffer, BLOCK_SIZE, 1, fh);

    return (int32_t) BLOCK_SIZE;
}

void p2d::io_flash_write_sector(uint32_t offset, const uint8_t *buffer) {
    fseek(fh, offset, SEEK_SET);
    fwrite(buffer, BLOCK_SIZE, 1, fh);
    fflush(fh);
}

void p2d::io_flash_exit() {
    if (fh) fclose(fh);
}
