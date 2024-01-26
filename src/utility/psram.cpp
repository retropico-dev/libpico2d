//
// Created by cpasjuste on 18/01/24.
//

#ifdef PICO_PSRAM

#include <hardware/clocks.h>
#include <hardware/pio.h>
#include "platform.h"
#include "psram.h"
#include "aps6404.hpp"
#include "pinout.h"

using namespace p2d;

static uint32_t current_address = 0;
static uint32_t max_address = 8 * 1024 * 1024;
static bool inited = false;

static pimoroni::APS6404 *ram;

bool PSram::init() {
    if (inited) return true;

    ram = new pimoroni::APS6404(PSRAM_PIN_CS, PSRAM_PIN_D0, PSRAM_PIO);
    ram->init();

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

void in_ram(PSram::read)(const uint32_t addr, uint32_t *read_buf, const size_t count) {
    ram->read_blocking(addr, read_buf, count);
}

void in_ram(PSram::write)(const uint32_t addr, uint32_t *data, const size_t count) {
    ram->write(addr, data, count);
    ram->wait_for_finish_blocking();
}

void in_ram(PSram::memset)(uint32_t addr, uint32_t value, size_t count) {
    for (uint_fast16_t i = 0; i < count; i++) {
        ram->write(addr, (uint32_t *) value, count);
    }
}

uint32_t in_ram(PSram::alloc)(size_t size) {
    if (current_address + size < max_address) {
        current_address += size;
        return current_address;
    }

    return PSRAM_INVALID_ADDRESS;
}

#endif