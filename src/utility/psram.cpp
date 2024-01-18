//
// Created by cpasjuste on 18/01/24.
//

#ifdef PICO_PSRAM

#define PSRAM_DEBUG 0

#include <hardware/clocks.h>
#include "platform.h"
#include "psram.h"
#include "psram_spi.h"

using namespace p2d;

static PIO pio = pio0;
static uint pio_sm = 1;
static uint32_t current_address = 0;
static uint32_t max_address = 8 * 1024 * 1024;
static bool inited = false;
psram_spi_inst_t p2d_psram_spi;
psram_spi_inst_t *async_spi_inst;

void PSram::init(float spi_mhz) {
    if (inited) return;

    // calculate clock divider
    auto sys_clock = (uint16_t) (clock_get_hz(clk_sys) / 1000000);
    auto clock_div = (float) sys_clock * (62.5f / spi_mhz) / 125;

    // pio/sm setup
    pio_sm_claim(pio, pio_sm);
    p2d_psram_spi = psram_spi_init_clkdiv(pio, (int) pio_sm, clock_div, true);

    // minimal psram test
    uint8_t write_buffer[12] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\0'};
    PSram::write(0, write_buffer, 12);
    uint8_t read_buffer[12];
    PSram::read(0, read_buffer, 12);
    if (memcmp(write_buffer, read_buffer, 12) != 0) {
        printf("PSram::init: WARNING: psram read/write test failed, check your setup...\r\n");
    } else {
        printf("PSram: psram initialization success @ %i Mhz\r\n", (uint16_t) spi_mhz);
    }

    inited = true;
}

void in_ram(PSram::read)(const uint32_t addr, uint8_t *dst, const size_t count) {
    // max 31 bytes per read
    size_t diff = count % 30;
#if PSRAM_DEBUG
    size_t read = 0;
#endif
    if (count > 29) {
        for (uint_fast16_t i = 0; i < count - diff; i += 30) {
#if PSRAM_DEBUG
            printf("PSram::read[%i]: addr == %lu\r\n", i, addr + i);
            read += 30;
#endif
            psram_read(&p2d_psram_spi, addr + i, dst + i, 30);
        }
    }

    // read remaining bytes
    if (diff) {
#if PSRAM_DEBUG
        printf("PSram::read[diff]: addr == %lu\r\n", addr + count - diff);
        read += diff;
#endif
        psram_read(&p2d_psram_spi, addr + count - diff, dst + count - diff, diff);
    }

#if PSRAM_DEBUG
    printf("PSram::read: read: %i, diff: %i\r\n", read, diff);
#endif
}

void in_ram(PSram::write)(const uint32_t addr, uint8_t *src, const size_t count) {
    // max 27 bytes per write
    size_t diff = count % 26;
#if PSRAM_DEBUG
    size_t wrote = 0;
#endif
    if (count > 25) {
        for (uint_fast16_t i = 0; i < count - diff; i += 26) {
#if PSRAM_DEBUG
            printf("PSram::write[%i]: addr == %lu\r\n", i, addr + i);
            wrote += 26;
#endif
            psram_write_async_fast(&p2d_psram_spi, addr + i, src + i, 26);
        }
    }

    // read remaining bytes
    if (diff) {
#if PSRAM_DEBUG
        printf("PSram::write[diff]: addr == %lu\r\n", addr + count - diff);
        wrote += diff;
#endif
        psram_write_async_fast(&p2d_psram_spi, addr + count - diff, src + count - diff, diff);
    }

#if PSRAM_DEBUG
    printf("PSram::write: wrote: %i, diff: %i\r\n", wrote, diff);
#endif
}

void in_ram(PSram::memset)(uint32_t addr, uint32_t value, size_t count) {
    for (uint_fast16_t i = 0; i < count; i++) {
        psram_write32_async(&p2d_psram_spi, addr + i, value);
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