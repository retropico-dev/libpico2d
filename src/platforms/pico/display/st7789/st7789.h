//
// Created by cpasjuste on 08/06/23.
//

#ifndef MICROBOY_ST7789_H
#define MICROBOY_ST7789_H

#define TEST_LINE_BUFFER 1

#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 240

// Delays
#define ST7789_RST_DELAY     120  // ms, wait before sending Sleep Out command
#define ST7789_SWRESET_DELAY 120  // ms, wait for software reset finish
#define ST7789_SLPOUT_DELAY  120  // ms, wait for sleep out finish

// System Function Command List - Write Commands Only
#define ST7789_NOP       0x00  // NOP
#define ST7789_SWRESET   0x01  // Software Reset
#define ST7789_SLPIN     0x10  // Sleep In
#define ST7789_SLPOUT    0x11  // Sleep Out
#define ST7789_PTLON     0x12  // Partial Display Mode On
#define ST7789_NORON     0x13  // Normal Display Mode On
#define ST7789_INVOFF    0x20  // Display Inversion Off
#define ST7789_INVON     0x21  // Display Inversion On
#define ST7789_DISPOFF   0x28  // Display Off
#define ST7789_DISPON    0x29  // Display On
#define ST7789_CASET     0x2A  // Column Address Set
#define ST7789_RASET     0x2B  // Row Address Set
#define ST7789_RAMWR     0x2C  // Memory Write
#define ST7789_PLTAR     0x30  // Partial Area
#define ST7789_MADCTL    0x36  // Memory Data Access Control
#define ST7789_COLMOD    0x3A  // Interface Pixel Format
#define ST7789_RAMCTRL   0xB0  // RAM Control
#define ST7789_PORCTRL   0xB2  // Porch Setting
#define ST7789_GCTRL     0xB7  // Gate Control
#define ST7789_VCOMS     0xBB  // VCOMS Setting
#define ST7789_LCMCTRL   0xC0  // LCM Control
#define ST7789_VDVVRHEN  0xC2  // VDV and VRH Command Enable
#define ST7789_VRHS      0xC3  // VRH Set
#define ST7789_VDVS      0xC4  // VDV Set
#define ST7789_FRCTRL2   0xC6  // Frame Rate Control in Normal Mode
#define ST7789_PWCTRL1   0xD0  // Power Control 1
#define ST7789_PVGAMCTRL 0xE0  // Positive Voltage Gamma Control
#define ST7789_NVGAMCTRL 0xE1  // Negative Voltage Gamma Control

// MADCTL Parameters
#define ST7789_MADCTL_MH  0x04  // Bit 2 - Refresh Left to Right
#define ST7789_MADCTL_RGB 0x00  // Bit 3 - RGB Order
#define ST7789_MADCTL_BGR 0x08  // Bit 3 - BGR Order
#define ST7789_MADCTL_ML  0x10  // Bit 4 - Scan Address Increase
#define ST7789_MADCTL_MV  0x20  // Bit 5 - X-Y Exchange
#define ST7789_MADCTL_MX  0x40  // Bit 6 - X-Mirror
#define ST7789_MADCTL_MY  0x80  // Bit 7 - Y-Mirror

// COLMOD Parameter
#define ST7789_COLOR_MODE_12BIT 0x53    //  RGB444 (12bit)
#define ST7789_COLOR_MODE_16BIT 0x55    //  RGB565 (16bit)
//#define ST7789_COLOR_MODE_18bit 0x66    //  RGB666 (18bit)

#define ST7789_ROTATION 2

#if ST7789_ROTATION == 0
#define X_SHIFT 0
#define Y_SHIFT 80
#elif ST7789_ROTATION == 1
#define X_SHIFT 80
#define Y_SHIFT 0
#elif ST7789_ROTATION == 2
#define X_SHIFT 0
#define Y_SHIFT 0
#elif ST7789_ROTATION == 3
#define X_SHIFT 0
#define Y_SHIFT 0
#endif

extern uint16_t frame_buffer[];

void st7789_init(uint8_t format, float clock_div);

void st7789_start_pixels();

void st7789_set_cursor(uint16_t x, uint16_t y);

void st7789_put8(uint16_t pixel);

void st7789_put16(uint16_t pixel);

void st7789_put32(uint32_t pixel);

void st7789_set_data_size(uint8_t size);

void st7789_prepare_write();

void st7789_push(uint32_t size = DISPLAY_WIDTH, bool dont_block = false);

void st7789_clear();

void st7789_dma_flush();

bool st7789_dma_is_busy();

#endif //MICROBOY_ST7789_H
