//
// Created by cpasjuste on 08/06/23.
//

#include <hardware/gpio.h>
#include <pico/time.h>
#include <cstdio>
#include <hardware/structs/clocks.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include "st7789-spi.pio.h"
#include "st7789.h"
#include "pinout.h"

static PIO pio = LCD_PIO;
static uint pio_sm = LCD_SM;
static uint8_t pio_bit_size = 16;
static uint32_t dma_channel = 0;

uint16_t *frame_buffer;

// used for pixel doubling
static void __isr

st7789_dma_irq_handler() {
    /*
    if (dma_channel_get_irq0_status(dma_channel)) {
        dma_channel_acknowledge_irq0(dma_channel);

        if (++cur_scanline > win_h / 2)
            return;

        auto count = cur_scanline == (win_h + 1) / 2 ? win_w / 4 : win_w / 2;

        dma_channel_set_trans_count(dma_channel, count, false);
        dma_channel_set_read_addr(dma_channel, upd_frame_buffer + (cur_scanline - 1) * (win_w / 2), true);
    }
    */
}

// Format: cmd length (including cmd byte), post delay in units of 5 ms, then cmd payload
// Note the delays have been shortened a little
static uint8_t st7789_init_seq[] = {
        1, 20, ST7789_SWRESET,      // Software reset
        1, 10, ST7789_SLPOUT,       // Exit sleep mode
        2, 2, ST7789_COLMOD, ST7789_COLOR_MODE_16BIT,  // Set colour mode to 16 bit (RGB565)
        2, 0, ST7789_MADCTL, ST7789_MADCTL_RGB,  // Set MADCTL
        5, 0, ST7789_CASET, 0x00, 0x00, DISPLAY_WIDTH >> 8, DISPLAY_WIDTH & 0xff,   // CASET: column addresses
        5, 0, ST7789_RASET, 0x00, 0x00, DISPLAY_HEIGHT >> 8, DISPLAY_HEIGHT & 0xff, // RASET: row addresses
        6, 1, ST7789_PORCTRL, 0x0c, 0x0c, 0x00, 0x33, 0x33,
        2, 1, ST7789_GCTRL, 0x14,
        2, 1, ST7789_VCOMS, 0x37,
        2, 1, ST7789_LCMCTRL, 0x2c,
        2, 1, ST7789_VDVVRHEN, 0x01,
        2, 1, ST7789_VRHS, 0x12,
        2, 1, ST7789_VDVS, 0x20,
        15, 1, ST7789_PVGAMCTRL, 0xD0, 0x08, 0x11, 0x08, 0x0c, 0x15, 0x39, 0x33, 0x50, 0x36, 0x13, 0x14, 0x29, 0x2d,
        15, 1, ST7789_NVGAMCTRL, 0xD0, 0x08, 0x10, 0x08, 0x06, 0x06, 0x39, 0x44, 0x51, 0x0b, 0x16, 0x14, 0x2f, 0x31,
        3, 1, ST7789_PWCTRL1, 0xa4, 0xa1, // Power Control 1 -> VDS = 2.3V, AVCL = -4.8V, AVDD = 6.8v ?
        //2, 1, ST7789_FRCTRL2, 0xf, // 0x15 -> 50Hz ? 0xf -> 60hz ?
        1, 2, ST7789_INVON,         // Inversion on, then 10 ms delay (supposedly a hack?)
        1, 2, ST7789_NORON,         // Normal display on, then 10 ms delay
        1, 2, ST7789_DISPON,        // Main screen turn on, then wait 500 ms
        0                           // Terminate list
};

static inline void st7789_lcd_set_dc_cs(bool dc, bool cs) {
    sleep_us(1);
    gpio_put_masked((1u << LCD_PIN_DC) | (1u << LCD_PIN_CS), !!dc << LCD_PIN_DC | !!cs << LCD_PIN_CS);
    sleep_us(1);
}

static inline void st7789_lcd_write_cmd(const uint8_t *cmd, size_t count) {
    st7789_lcd_wait_idle(pio, pio_sm);
    st7789_lcd_set_dc_cs(false, false);
    st7789_lcd_set_autopull_threshold(pio, pio_sm, 8); // switch to 8 bit mode
    st7789_lcd_put8(pio, pio_sm, *cmd++);
    if (count >= 2) {
        st7789_lcd_wait_idle(pio, pio_sm);
        st7789_lcd_set_dc_cs(true, false);
        for (size_t i = 0; i < count - 1; ++i)
            st7789_lcd_put8(pio, pio_sm, *cmd++);
    }
    st7789_lcd_wait_idle(pio, pio_sm);
    st7789_lcd_set_dc_cs(true, true);

    // switch back to screen mode
    st7789_lcd_set_autopull_threshold(pio, pio_sm, pio_bit_size);
}

static inline void st7789_lcd_write_cmd(uint8_t cmd) {
    st7789_lcd_wait_idle(pio, pio_sm);
    st7789_lcd_set_dc_cs(false, false);
    st7789_lcd_set_autopull_threshold(pio, pio_sm, 8); // switch to 8 bit mode
    st7789_lcd_put8(pio, pio_sm, cmd);
    st7789_lcd_wait_idle(pio, pio_sm);
    st7789_lcd_set_dc_cs(true, true);

    // switch back to screen mode
    st7789_lcd_set_autopull_threshold(pio, pio_sm, pio_bit_size);
}

static inline void st7789_lcd_init(const uint8_t *init_seq) {
    const uint8_t *cmd = init_seq;
    while (*cmd) {
        st7789_lcd_write_cmd(cmd + 2, *cmd);
        sleep_ms(*(cmd + 1) * 5);
        cmd += *cmd + 2;
    }
}

static inline void st7789_lcd_set_cursor(uint16_t x, uint16_t y) {
    const uint8_t st7789_cursor_seq[] = {
            5, 0, ST7789_CASET, (uint8_t) (x >> 8), (uint8_t) (x & 0xff), DISPLAY_WIDTH >> 8, DISPLAY_WIDTH & 0xff,
            5, 0, ST7789_RASET, (uint8_t) (y >> 8), (uint8_t) (y & 0xff), DISPLAY_HEIGHT >> 8, DISPLAY_HEIGHT & 0xff,
            0
    };

    const uint8_t *cmd = st7789_cursor_seq;
    while (*cmd) {
        st7789_lcd_write_cmd(cmd + 2, *cmd);
        cmd += *cmd + 2;
    }

    // enable write
    st7789_lcd_write_cmd(ST7789_RAMWR);
    st7789_lcd_set_dc_cs(true, false);
}

void st7789_init(uint8_t format, float spiClockMhz) {
    // 62.5f = pico default @ 125 Mhz sys clock (safe)
    // tested working at 88.6 Mhz @ 266 Mhz clock (unsafe)
    auto sys_clock = (uint16_t) (clock_get_hz(clk_sys) / 1000000);
    auto clock_div = (float) sys_clock * (62.5f / spiClockMhz) / 125;

    uint offset = pio_add_program(pio, &st7789_raw_program);
    //pio_sm_claim(pio, pio_sm);
    st7789_lcd_program_init(pio, pio_sm, offset, LCD_PIN_DIN, LCD_PIN_CLK, clock_div);

    gpio_init(LCD_PIN_CS);
    gpio_init(LCD_PIN_DC);
    gpio_init(LCD_PIN_RESET);
    gpio_init(LCD_PIN_BL);
    gpio_set_dir(LCD_PIN_CS, GPIO_OUT);
    gpio_set_dir(LCD_PIN_DC, GPIO_OUT);
    gpio_set_dir(LCD_PIN_RESET, GPIO_OUT);
    gpio_set_dir(LCD_PIN_BL, GPIO_OUT);

    gpio_put(LCD_PIN_CS, true);
    gpio_put(LCD_PIN_RESET, true);

    pio_bit_size = format == ST7789_COLOR_MODE_12BIT ? 12 : 16;
    st7789_init_seq[9] = format;
    st7789_lcd_init(st7789_init_seq);

    gpio_put(LCD_PIN_BL, true);

    // initialise dma channel for transmitting pixel data to screen
    dma_channel = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(dma_channel);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_16);
    channel_config_set_dreq(&config, pio_get_dreq(pio, pio_sm, true));
#if TEST_LINE_BUFFER
    dma_channel_configure(dma_channel, &config, &pio->txf[pio_sm], frame_buffer, DISPLAY_WIDTH, false);
#else
    dma_channel_configure(dma_channel, &config, &pio->txf[pio_sm], frame_buffer,
                          DISPLAY_WIDTH * DISPLAY_HEIGHT, false);
#endif
    irq_add_shared_handler(DMA_IRQ_0, st7789_dma_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    irq_set_enabled(DMA_IRQ_0, true);

    st7789_clear();
}

void st7789_start_pixels() {
    st7789_lcd_write_cmd(ST7789_RAMWR);
    st7789_lcd_set_dc_cs(true, false);
}

void st7789_put8(uint16_t pixel) {
    st7789_lcd_put8(pio, pio_sm, pixel);
}

void st7789_put16(uint16_t pixel) {
    st7789_lcd_put16(pio, pio_sm, pixel);
}

void st7789_put32(uint32_t pixel) {
    st7789_lcd_put32(pio, pio_sm, pixel);
}

void st7789_set_cursor(uint16_t x, uint16_t y) {
    st7789_lcd_set_cursor(x, y);
}

void st7789_set_data_size(uint8_t size) {
    pio_bit_size = size;
    st7789_lcd_wait_idle(pio, pio_sm);
    st7789_lcd_set_autopull_threshold(pio, pio_sm, size);
}


////////// DMA TEST ///////////

#define RAMWR 0x2C
bool write_mode = false;
bool pixel_double = false;

static void pio_put_byte(PIO p, uint sm, uint8_t b) {
    while (pio_sm_is_tx_fifo_full(p, sm));
    *(volatile uint8_t *) &p->txf[sm] = b;
}

static void pio_wait(PIO p, uint sm) {
    uint32_t stall_mask = 1u << (PIO_FDEBUG_TXSTALL_LSB + sm);
    p->fdebug |= stall_mask;
    while (!(p->fdebug & stall_mask));
}

void st7789_prepare_write() {
    pio_wait(pio, pio_sm);

    // setup for writing
    uint8_t r = RAMWR;
    gpio_put(LCD_PIN_CS, false);

    gpio_put(LCD_PIN_DC, false); // command mode
    pio_put_byte(pio, pio_sm, r);
    pio_wait(pio, pio_sm);

    gpio_put(LCD_PIN_DC, true); // data mode

    pio_sm_set_enabled(pio, pio_sm, false);
    pio_sm_restart(pio, pio_sm);

    if (pixel_double) {
        /*
        // switch program
        pio_sm_set_wrap(pio, pio_sm, pio_double_offset + st7789_pixel_double_wrap_target,
                        pio_double_offset + st7789_pixel_double_wrap);

        // 32 bits, no autopull
        pio->sm[pio_sm].shiftctrl &= ~(PIO_SM0_SHIFTCTRL_PULL_THRESH_BITS | PIO_SM0_SHIFTCTRL_AUTOPULL_BITS);

        pio_sm_exec(pio, pio_sm, pio_encode_jmp(pio_double_offset));

        // reconfigure dma size
        dma_channel_hw_addr(dma_channel)->al1_ctrl &= ~DMA_CH0_CTRL_TRIG_DATA_SIZE_BITS;
        dma_channel_hw_addr(dma_channel)->al1_ctrl |= DMA_SIZE_32 << DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB;
        */
    } else {
        // 16 bits, autopull
        pio->sm[pio_sm].shiftctrl &= ~PIO_SM0_SHIFTCTRL_PULL_THRESH_BITS;
        pio->sm[pio_sm].shiftctrl |= (16 << PIO_SM0_SHIFTCTRL_PULL_THRESH_LSB) | PIO_SM0_SHIFTCTRL_AUTOPULL_BITS;

        dma_channel_hw_addr(dma_channel)->al1_ctrl &= ~DMA_CH0_CTRL_TRIG_DATA_SIZE_BITS;
        dma_channel_hw_addr(dma_channel)->al1_ctrl |= DMA_SIZE_16 << DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB;
    }

    pio_sm_set_enabled(pio, pio_sm, true);

    write_mode = true;
}

void st7789_push(uint16_t *data, uint32_t size, bool dont_block) {
    if (dma_channel_is_busy(dma_channel) && dont_block) {
        return;
    }

    dma_channel_wait_for_finish_blocking(dma_channel);

    if (!write_mode)
        st7789_prepare_write();

    /*
    if (pixel_double) {
        cur_scanline = 0;
        //upd_frame_buffer = frame_buffer; // TODO
        dma_channel_set_trans_count(dma_channel, win_w / 4, false);
    } else {
        dma_channel_set_trans_count(dma_channel, size, false);
    }
    */
    dma_channel_set_trans_count(dma_channel, size, false);
    dma_channel_set_read_addr(dma_channel, data, true);
}

void st7789_clear() {
    if (!write_mode)
        st7789_prepare_write();

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
        pio_sm_put_blocking(pio, pio_sm, 0);
}

bool st7789_dma_is_busy() {
    //if (pixel_double && cur_scanline <= win_h / 2)
    //    return true;
    return dma_channel_is_busy(dma_channel);
}

void st7789_dma_flush() {
    while (dma_channel_is_busy(dma_channel)) {}
}
