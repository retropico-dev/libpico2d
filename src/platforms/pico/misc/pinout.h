//
// Created by cpasjuste on 14/06/23.
//

#ifndef P2D_PINOUT_H
#define P2D_PINOUT_H

// SPI LCD PINOUT
#define LCD_PIO         pio0
#define LCD_SM          0
#define LCD_PIN_CLK     (GPIO_PIN_LCD_CLK)   // SCK
#define LCD_PIN_DIN     (GPIO_PIN_LCD_DIN)   // MOSI
#define LCD_PIN_DC      (GPIO_PIN_LCD_DC)
#define LCD_PIN_CS      (GPIO_PIN_LCD_CS)
#define LCD_PIN_RESET   (GPIO_PIN_LCD_RST)
#define LCD_PIN_BL      (GPIO_PIN_LCD_BL)

// SPI SDCARD PINOUT
#define SD_PIO          pio1
#define SD_SM           0
#define SD_PIN_CS       (GPIO_PIN_SD_CS)
#define SD_PIN_CLK      (GPIO_PIN_SD_CLK)
#define SD_PIN_MOSI     (GPIO_PIN_SD_MOSI)
#define SD_PIN_MISO     (GPIO_PIN_SD_MISO)

// AUDIO PINOUT (I2S)
#define AUDIO_PIO       pio1
#define AUDIO_SM        1
#define AUDIO_PIN_DATA  (GPIO_PIN_AUDIO_DATA)  // DIN
#define AUDIO_PIN_CLOCK (GPIO_PIN_AUDIO_CLOCK)  // BCLK
#define AUDIO_PIN_LRC   (GPIO_PIN_AUDIO_LRC)  // LRC (CLOCK + 1)

#if PICO_PSRAM_RP2040
// PSRAM PINOUT (I2S)
#define PSRAM_PIO       pio1
#define PSRAM_SM        1
#define PSRAM_PIN_CS    17
#define PSRAM_PIN_CLK   18
#define PSRAM_PIN_D0    19
#define PSRAM_PIN_D1    20
#define PSRAM_PIN_D2    21
#define PSRAM_PIN_D3    22
#endif

#define BTN_PIN_VOL_U   (GPIO_PIN_BTN_VOL_UP)     // volume up
#define BTN_PIN_VOL_D   (GPIO_PIN_BTN_VOL_DOWN)     // volume down
#define BTN_PIN_UP      (GPIO_PIN_BTN_UP)
#define BTN_PIN_LEFT    (GPIO_PIN_BTN_LEFT)
#define BTN_PIN_DOWN    (GPIO_PIN_BTN_DOWN)
#define BTN_PIN_RIGHT   (GPIO_PIN_BTN_RIGHT)
#define BTN_PIN_SELECT  (GPIO_PIN_BTN_SELECT)
#define BTN_PIN_START   (GPIO_PIN_BTN_START)
#define BTN_PIN_A       (GPIO_PIN_BTN_A)
#define BTN_PIN_B       (GPIO_PIN_BTN_B)

#if !defined(PICO_DEBUG_UART)
#if GPIO_PIN_BTN_SLEEP > -1
#define BTN_PIN_SLEEP   (GPIO_PIN_BTN_SLEEP)    // sleep (dormant) mode
#endif
#endif

// UART (PICOPROBE)
#if defined(PICO_DEBUG_UART)
#define UART_TX         16
#define UART_RX         17
#endif

#endif //P2D_PINOUT_H
