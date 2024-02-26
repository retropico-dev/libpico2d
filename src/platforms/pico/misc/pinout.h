//
// Created by cpasjuste on 14/06/23.
//

#ifndef P2D_PINOUT_H
#define P2D_PINOUT_H

#if defined(RETROPICO_10)
// SPI LCD PINS         GPIO                        FEATHER RP2040
#define LCD_PIN_DC      8   // SPI1 RX              6
#define LCD_PIN_CS      9   // SPI1 CSn             9
#define LCD_PIN_CLK     10  // SPI1 SCK             10
#define LCD_PIN_DIN     11  // SPI1 TX (MOSI)       11
#define LCD_PIN_BL      12  // GPI0                 12
#define LCD_PIN_RESET   13  // GPIO                 13

// SPI SDCARD PINS
#define SD_PIN_CS       6   // SPI0 CSn             D4
#define SD_PIN_CLK      18  // SPI0 SCK             SCK
#define SD_PIN_MOSI     19  // SPI0 TX              MO
#define SD_PIN_MISO     20  // SPI0 RX              MI

// AUDIO PINS (I2S, DIGITAL)
#define AUDIO_PIN_DATA  26  // DIN                  A0
#define AUDIO_PIN_CLOCK 27  // BCLK                 A1
#define AUDIO_PIN_LRC   28  // LRC (CLOCK + 1)      A2

// BUTTONS PINS
#if !defined(NDEBUG) && defined(PICO_DEBUG_UART)
#define BTN_PIN_A       (-1)    //                  25
#define BTN_PIN_B       (-1)    //                  24
#define BTN_PIN_START   (-1)    //                  RX
#define BTN_PIN_SELECT  (-1)    //                  TX
#define BTN_PIN_LEFT    (-1)    //                  SCL
#define BTN_PIN_RIGHT   (-1)    //                  A3
#define BTN_PIN_UP      (-1)    //                  5
#define BTN_PIN_DOWN    (-1)    //                  SDA
#else
#define BTN_PIN_A       (25)    //                  25
#define BTN_PIN_B       (24)    //                  24
#define BTN_PIN_START   (1)     //                  RX
#define BTN_PIN_SELECT  (0)     //                  TX
#define BTN_PIN_LEFT    (3)     //                  SCL
#define BTN_PIN_RIGHT   (29)    //                  A3
#define BTN_PIN_UP      (7)     //                  5
#define BTN_PIN_DOWN    (2)     //                  SDA
#endif
#elif defined(RETROPICO_11)

// SPI LCD PINOUT
#define LCD_PIO         pio0
#define LCD_SM          0
#define LCD_PIN_CLK     5   // SCK
#define LCD_PIN_DIN     4   // MOSI
#define LCD_PIN_DC      2
#define LCD_PIN_CS      1
#define LCD_PIN_RESET   3
#define LCD_PIN_BL      0

// SPI SDCARD PINOUT
#define SD_PIO          pio1
#define SD_SM           0
#define SD_PIN_CS       26
#define SD_PIN_CLK      22
#define SD_PIN_MOSI     21
#define SD_PIN_MISO     27

// AUDIO PINOUT (I2S)
#define AUDIO_PIO       pio1
#define AUDIO_SM        1
#define AUDIO_PIN_DATA  20  // DIN
#define AUDIO_PIN_CLOCK 18  // BCLK
#define AUDIO_PIN_LRC   19  // LRC (CLOCK + 1)

#if PICO_PSRAM
// PSRAM PINOUT
#define PSRAM_PIO       pio1
#define PSRAM_SM        1
#define PSRAM_PIN_CS    17
#define PSRAM_PIN_CLK   18
#define PSRAM_PIN_D0    19
#define PSRAM_PIN_D1    20
#define PSRAM_PIN_D2    21
#define PSRAM_PIN_D3    22
#endif

#define BTN_PIN_VOL_U   (6)     // volume up
#define BTN_PIN_VOL_D   (7)     // volume down
#define BTN_PIN_UP      (8)
#define BTN_PIN_LEFT    (9)
#define BTN_PIN_DOWN    (10)
#define BTN_PIN_RIGHT   (11)
#define BTN_PIN_START   (12)
#define BTN_PIN_SELECT  (13)
#define BTN_PIN_A       (15)
#define BTN_PIN_B       (14)

#if !defined(PICO_DEBUG_UART)
#define BTN_PIN_SLEEP   (17)    // sleep (dormant) mode
#endif

// UART (PICOPROBE)
#if defined(PICO_DEBUG_UART)
#define UART_TX         16
#define UART_RX         17
#endif

#else
#error "pinout.h: target device not defined"
#endif

#endif //P2D_PINOUT_H
