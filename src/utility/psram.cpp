//
// Created by cpasjuste on 18/01/24.
//

#ifdef PICO_PSRAM

#include <hardware/clocks.h>
#include <hardware/pio.h>
#include "platform.h"
#include "psram.h"
#include "pinout.h"

using namespace p2d;

static uint32_t current_address = 0;
static uint32_t max_address = 8 * 1024 * 1024;
static bool inited = false;
static pimoroni::APS6404 *p_ram;

bool PSram::init() {
    if (inited) return true;

    p_ram = new pimoroni::APS6404(PSRAM_PIN_CS, PSRAM_PIN_D0, PSRAM_PIO);
    p_ram->init();

    // minimal psram test
    uint8_t write_buffer[10] = {'R', 'e', 't', 'r', 'o', 'P', 'i', 'c', 'o', '\0'};
    for (uint32_t i = 0; i < 32; i++) {
        write(i * 10, (uint32_t *) write_buffer, 10);
    }

    uint32_t read_buffer[10];
    for (uint32_t i = 0; i < 32; i++) {
        std::memset(read_buffer, 0, 10);
        read(i * 10, (uint32_t *) read_buffer, 10);
        if (memcmp(write_buffer, read_buffer, 10) != 0) {
            printf("PSram::init: WARNING: psram read/write test failed, check your setup...\r\n");
            return false;
        }
    }

    printf("PSram: psram initialization success...\r\n");

    inited = true;
    return true;
}

pimoroni::APS6404 *PSram::getRam() {
    return p_ram;
}

void in_ram(PSram::read)(const uint32_t addr, uint32_t *read_buf, const size_t len) {
    p_ram->read(addr, read_buf, len);
}

void in_ram(PSram::read)(uint32_t addr, uint16_t *read_buf, size_t len) {
    if ((uintptr_t) read_buf & 1) {
        uint32_t tmp;
        p_ram->read_blocking(addr, &tmp, 1);
        *read_buf++ = tmp & 0xFFFF;
        len--;
    }

    if ((len >> 1) > 0) {
        p_ram->read_blocking(addr, (uint32_t *) read_buf, len >> 1);
    }

    if (len & 1) {
        uint32_t tmp;
        p_ram->read_blocking(addr, &tmp, 1);
        read_buf[len - 1] = tmp & 0xFFFF;
    }
}

void in_ram(PSram::read_blocking)(const uint32_t addr, uint32_t *read_buf, const size_t len) {
    p_ram->read_blocking(addr, read_buf, len);
}

void in_ram(PSram::write)(const uint32_t addr, uint32_t *data, const size_t len) {
    p_ram->write(addr, data, len);
}

void in_ram(PSram::write)(uint32_t address, const uint16_t color, const size_t len) {
    uint32_t val = color | ((uint32_t) color << 16);
    p_ram->write_repeat(address, val, len << 1);
}

void in_ram(PSram::flush)() {
    p_ram->wait_for_finish_blocking();
}

void in_ram(PSram::memset)(uint32_t addr, uint32_t value, size_t len) {
    for (uint_fast16_t i = 0; i < len; i++) {
        p_ram->write(addr, (uint32_t *) value, len);
    }
}

uint32_t in_ram(PSram::alloc)(size_t size, bool clear) {
    if (current_address + size < max_address) {
        current_address += size;
        if (clear) {
            p_ram->write_repeat(current_address, 0, size << 1);
        }
        return current_address;
    }

    return PSRAM_INVALID_ADDRESS;
}

#endif
