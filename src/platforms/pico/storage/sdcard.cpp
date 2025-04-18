//
// Created by cpasjuste on 19/01/24.
//

// source: https://github.com/32blit/32blit-sdk/blob/master/32blit-pico/storage_sd_spi.cpp

#include <cstdio>
#include <pico/time.h>
#include <pico/binary_info.h>
#include <hardware/pio.h>
#include <hardware/clocks.h>

#include "utility.h"
#include "sdcard.h"
#include "pinout.h"
#include "spi.pio.h"

#define SD_TIMEOUT 10

static PIO sd_pio = SD_PIO;
static int sd_sm = SD_SM;
static bool sd_io_initialised = false;

static uint32_t card_size_blocks = 0;
static bool is_hcs = false;

static void spi_write(const uint8_t *buf, size_t len) {
    size_t tx_remain = len, rx_remain = len;
    auto tx_fifo = (io_rw_8 *) &sd_pio->txf[sd_sm];
    auto rx_fifo = (io_rw_8 *) &sd_pio->rxf[sd_sm];

    while (tx_remain || rx_remain) {
        if (tx_remain && !pio_sm_is_tx_fifo_full(sd_pio, sd_sm)) {
            *tx_fifo = *buf++;
            --tx_remain;
        }
        if (rx_remain && !pio_sm_is_rx_fifo_empty(sd_pio, sd_sm)) {
            (void) *rx_fifo;
            --rx_remain;
        }
    }
}

static void spi_read(uint8_t *buf, size_t len) {
    size_t tx_remain = len, rx_remain = len;
    auto tx_fifo = (io_rw_8 *) &sd_pio->txf[sd_sm];
    auto rx_fifo = (io_rw_8 *) &sd_pio->rxf[sd_sm];

    while (tx_remain || rx_remain) {
        if (tx_remain && !pio_sm_is_tx_fifo_full(sd_pio, sd_sm)) {
            *tx_fifo = 0xFF;
            --tx_remain;
        }
        if (rx_remain && !pio_sm_is_rx_fifo_empty(sd_pio, sd_sm)) {
            *buf++ = *rx_fifo;
            --rx_remain;
        }
    }
}

static uint8_t spi_transfer_byte(uint8_t b) {
    while (pio_sm_is_tx_fifo_full(sd_pio, sd_sm));
    *(io_rw_8 *) &sd_pio->txf[sd_sm] = b;

    while (pio_sm_is_rx_fifo_empty(sd_pio, sd_sm));
    return *(io_rw_8 *) &sd_pio->rxf[sd_sm];
}

static bool sd_wait_ready() {
    absolute_time_t timeout_time = make_timeout_time_ms(SD_TIMEOUT);

    while (spi_transfer_byte(0xFF) != 0xFF) {
        if (absolute_time_diff_us(get_absolute_time(), timeout_time) <= 0)
            return false;
    }

    return true;
}

static bool sd_begin() {
    gpio_put(SD_PIN_CS, false);

    // wait for ready
    if (!sd_wait_ready()) {
        gpio_put(SD_PIN_CS, true);
        return false;
    }

    return true;
}

static void sd_end() {
    gpio_put(SD_PIN_CS, true);
    spi_transfer_byte(0xFF);
}

static void sd_write_command(uint8_t cmd, uint32_t param, uint8_t crc) {
    uint8_t buf[]{
        uint8_t(0x40 | cmd),
        uint8_t(param >> 24),
        uint8_t(param >> 16),
        uint8_t(param >> 8),
        uint8_t(param),
        crc
    };

    spi_write(buf, sizeof(buf));
}

uint8_t sd_read_response() {
    uint8_t ret;
    int attempt = 0;
    while (((ret = spi_transfer_byte(0xFF)) & 0x80) && (attempt++ < 8));

    return ret;
}

static uint8_t sd_command1(uint8_t cmd, uint32_t param, uint8_t crc = 1) {
    if (!sd_begin())
        return 0xFF;

    sd_write_command(cmd, param, crc);
    uint8_t res = sd_read_response();

    sd_end();
    return res;
}

static uint8_t sd_command3_7(uint8_t cmd, uint32_t param, uint32_t &res_data, uint8_t crc = 1) {
    if (!sd_begin())
        return 0xFF;

    sd_write_command(cmd, param, crc);

    uint8_t res = sd_read_response();

    // no error bits
    if ((res & 0xFE) == 0) {
        spi_read((uint8_t *) &res_data, 4);
        res_data = __builtin_bswap32(res_data);
    }

    sd_end();
    return res;
}

static bool sd_read_block(uint8_t *buffer, int len) {
    // wait for start token
    absolute_time_t timeout_time = make_timeout_time_ms(SD_TIMEOUT * 10);

    while (spi_transfer_byte(0xFF) != 0xFE) {
        if (absolute_time_diff_us(get_absolute_time(), timeout_time) <= 0)
            return false;
    }

    spi_read(buffer, len);

    // crc
    spi_transfer_byte(0xFF);
    spi_transfer_byte(0xFF);

    return true;
}

static uint8_t sd_command_read_block(uint8_t cmd, uint32_t addr, uint8_t *buffer) {
    if (!sd_begin())
        return 0xFF;

    sd_write_command(cmd, addr, 1);

    uint8_t res = sd_read_response();

    if (res == 0) {
        int len = 512;
        if (cmd == 9 || cmd == 10) // CSD/CID are 16 bytes
            len = 16;
        else if (cmd == 6) // SWITCH
            len = 64;

        if (!sd_read_block(buffer, len))
            return 0xFF;
    }

    sd_end();

    return res;
}

static uint8_t sd_command_read_block_multiple(uint8_t cmd, uint32_t addr, uint8_t *buffer, int count, uint32_t &read) {
    if (!sd_begin())
        return 0xFF;

    sd_write_command(cmd, addr, 1);

    uint8_t res = sd_read_response();

    if (res == 0) {
        // likely only used with CMD 18
        const int len = 512;

        while (count--) {
            if (!sd_read_block(buffer, len))
                return 0xFF;

            read += len;
            buffer += len;
        }

        // now CMD12 to end
        sd_write_command(12, 0, 1); // STOP_TRANSMISSION
        spi_transfer_byte(0xFF); // stuff byte
        sd_read_response();
    }

    sd_end();

    return res;
}

static uint8_t sd_command_write_block(uint8_t cmd, uint32_t addr, const uint8_t *buffer) {
    if (!sd_begin())
        return 0xFF;

    sd_write_command(cmd, addr, 1);

    uint8_t res = sd_read_response();

    if (res == 0) {
        if (!sd_wait_ready()) {
            sd_end();
            return 0xFF;
        }

        spi_transfer_byte(0xFE); // start token (different for CMD25)

        int len = 512;

        for (int i = 0; i < len; i++)
            spi_transfer_byte(*buffer++);

        // crc
        spi_transfer_byte(0xFF);
        spi_transfer_byte(0xFF);

        auto dataRes = spi_transfer_byte(0xFF) & 0x1F;

        // check accepted
        if (dataRes != 0x5) {
            sd_end();
            return 0xFF;
        }

        // wait for not busy
        while (spi_transfer_byte(0xFF) == 0);
    }

    sd_end();
    return res;
}

bool p2d::io_sdcard_init(float spiMhz) {
#if GPIO_PIN_SD_CS < 0 || GPIO_PIN_SD_CLK < 0 || GPIO_PIN_SD_MOSI < 0 || GPIO_PIN_SD_MISO < 0
    return false;
#else
    bi_decl(bi_4pins_with_names(SD_PIN_MISO, "SD RX", SD_PIN_MOSI, "SD TX", SD_PIN_CLK, "SD SCK", SD_PIN_CS, "SD CS"));

    // this will be called again if it fails
    if (!sd_io_initialised) {
        uint offset = pio_add_program(sd_pio, &spi_cpha0_program);
        //sd_sm = pio_claim_unused_sm(sd_pio, true);
        //pio_sm_claim(sd_pio, sd_sm);

        pio_sm_config c = spi_cpha0_program_get_default_config(offset);

        sm_config_set_out_pins(&c, SD_PIN_MOSI, 1);
        sm_config_set_in_pins(&c, SD_PIN_MISO);
        sm_config_set_sideset_pins(&c, SD_PIN_CLK);

        sm_config_set_out_shift(&c, false, true, 8);
        sm_config_set_in_shift(&c, false, true, 8);

        // MOSI, SCK output are low, MISO is input
        pio_sm_set_pins_with_mask(sd_pio, sd_sm, 0, (1u << SD_PIN_CLK) | (1u << SD_PIN_MOSI));
        pio_sm_set_pindirs_with_mask(sd_pio, sd_sm, (1u << SD_PIN_CLK) | (1u << SD_PIN_MOSI),
                                     (1u << SD_PIN_CLK) | (1u << SD_PIN_MOSI) | (1u << SD_PIN_MISO));
        pio_gpio_init(sd_pio, SD_PIN_MOSI);
        pio_gpio_init(sd_pio, SD_PIN_MISO);
        pio_gpio_init(sd_pio, SD_PIN_CLK);

        gpio_pull_up(SD_PIN_MISO);

        // SPI is synchronous, so bypass input synchronizer to reduce input delay.
        hw_set_bits(&sd_pio->input_sync_bypass, 1u << SD_PIN_MISO);

        pio_sm_init(sd_pio, sd_sm, offset, &c);
        pio_sm_set_enabled(sd_pio, sd_sm, true);

        // CS
        gpio_init(SD_PIN_CS);
        gpio_set_dir(SD_PIN_CS, GPIO_OUT);
        gpio_put(SD_PIN_CS, true);

        sd_io_initialised = true;
    }

    // go slow for init
    pio_sm_set_clkdiv(sd_pio, sd_sm, 250);

    uint8_t buf[]{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    spi_write(buf, sizeof(buf));

    // send cmd0
    uint8_t res = 0xFF;
    for (int retry = 0; retry < 16 && res == 0xFF; retry++)
        res = sd_command1(0, 0, 0x95); // GO_IDLE_STATE

    if (res != 0x1) {
        printf("io_sdcard_init: CMD0 failed (res %X)\r\n", res);
        return false;
    }

    // check voltage range / check if SDv2
    bool is_v2 = true;

    uint32_t data;
    res = sd_command3_7(8, 0x1AA, data, 0x87); // SEND_IF_COND

    if (res == 5) {
        // not supported, old card
        is_v2 = false;
    } else if (res != 1) {
        printf("io_sdcard_init: CMD8 failed (res %X)\r\n", res);
        return false;
    } else if (data != 0x1AA) {
        printf("io_sdcard_init: CMD8 returned unexpected %lX!\r\n", data);
        return false;
    }

    // init
    while (res != 0) {
        res = sd_command1(55, 0, 0x65); // APP_CMD

        if (res == 0x1)
            res = sd_command1(41, is_v2 ? 0x40000000 : 0, is_v2 ? 0x77 : 0xE5); // APP_SEND_OP_COND
        else if (res != 0xFF) {
            printf("io_sdcard_init: CMD55 failed (res %X)\r\n", res);
            return false;
        }

        if (res > 1 && res != 0xFF) {
            printf("io_sdcard_init: ACMD41 failed (res %X)\r\n", res);
            return false;
        }
    }

    // defaults, but make sure
    sd_command1(59, 0); // CRC_ON_OFF
    sd_command1(16, 512); // SET_BLOCKLEN

    // read OCR
    res = sd_command3_7(58, 0, data); // READ_OCR
    if (res != 0) {
        printf("io_sdcard_init: CMD58 failed (res %X)\r\n", res);
        return false;
    }

    is_hcs = false;
    if ((data & 0xC0000000) == 0xC0000000)
        is_hcs = true;

    // read CSD
    uint8_t csd[16];
    res = sd_command_read_block(9, 0, csd); // SEND_CSD

    if (res != 0) {
        printf("io_sdcard_init: CMD9 failed (res %X)\r\n", res);
        return false;
    }

    // v1
    if ((csd[0] >> 6) == 0) {
        int c_size = ((csd[6] & 0x3) << 10) | (csd[7] << 2) | (csd[8] >> 6);
        int c_size_mult = ((csd[9] & 0x3) << 1) | (csd[10] >> 7);
        int readBlLen = csd[5] & 0xF;

        uint32_t num_blocks = uint32_t(c_size + 1) * (2 << (c_size_mult + 1));
        uint32_t size_bytes = num_blocks * (2 << (readBlLen - 1));
        card_size_blocks = size_bytes / 512;
    } else { // v2
        // measured in 512k blocks
        card_size_blocks = (((int32_t(csd[7] & 0x3F) << 16) | (uint32_t(csd[8]) << 8) | csd[9]) + 1) * 1024;
    }

    // 62.5f = pico default @ 125 Mhz sys clock
    auto sys_clock = (uint16_t) (clock_get_hz(clk_sys) / 1000000);
    auto clock_div = (float) sys_clock * (62.5f / spiMhz) / 125;

    // attempt high speed
    uint8_t switch_res[64];
    if (sd_command_read_block(6, 0x80FFFFF1, switch_res) == 0) { // SWITCH
        if ((switch_res[16] & 0xF) == 1)
            clock_div /= 2; // successful switch, double speed
    }

    pio_sm_set_clkdiv(sd_pio, sd_sm, clock_div);
    pio_sm_restart(sd_pio, sd_sm);

    printf("Io: detected %s card @ %i Mhz \r\n",
           is_v2 ? (is_hcs ? "SDHC" : "SDv2") : "SDv1", (uint16_t) spiMhz);

    return true;
#endif
}

void p2d::io_sdcard_get_size(uint16_t &block_size, uint32_t &num_blocks) {
    block_size = 512;
    num_blocks = card_size_blocks;
}

uint64_t p2d::io_sdcard_get_size() {
    return (uint64_t) card_size_blocks * 512;
}

std::string p2d::io_sdcard_get_size_string() {
    return p2d::Utility::parseSize((uint64_t) card_size_blocks * 512);
}

int32_t p2d::io_sdcard_read(uint32_t sector, uint32_t offset, void *buffer, uint32_t size_bytes) {
    if (!is_hcs)
        sector *= 512;

    auto blocks = size_bytes / 512;

    if (blocks == 1) {
        if (sd_command_read_block(17, sector, (uint8_t *) buffer) != 0) // READ_SINGLE_BLOCK
            return 0;
        return (int32_t) size_bytes;
    } else {
        uint32_t read = 0;
        sd_command_read_block_multiple(18, sector, (uint8_t *) buffer, (int) blocks, read);
        return (int32_t) read;
    }
}

int32_t p2d::io_sdcard_write(uint32_t sector, uint32_t offset, const uint8_t *buffer, uint32_t size_bytes) {
    if (!is_hcs)
        sector *= 512;

    auto blocks = size_bytes / 512;

    int32_t written = 0;

    // TODO: multi block writes
    while (blocks--) {
        if (sd_command_write_block(24, sector, (uint8_t *) buffer + written) != 0) // WRITE_SINGLE_BLOCK
            break;

        written += 512;
        if (!is_hcs)
            sector += 512;
        else
            sector++;
    }

    return written;
}

void p2d::io_sdcard_exit() {
}
