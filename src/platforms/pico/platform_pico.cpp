//
// Created by cpasjuste on 30/05/23.
//

#include <pico/time.h>
#include <pico/stdlib.h>
#include <pico/bootrom.h>
#include <hardware/vreg.h>
#include <hardware/clocks.h>
#include <hardware/watchdog.h>
#include "platform.h"
#include "platform_pico.h"

using namespace p2d;

// ??!!
extern bool stdio_usb_connected();

#if defined(PICO_RP2350) && defined(GPIO_PIN_PSRAM_CS)
extern "C" uint8_t __psram_start__;
#endif

PicoPlatform::PicoPlatform(const Display::Settings &displaySettings) : Platform(displaySettings) {
    // first thing to do (reboot bootloader on key down)
    p_input = new PicoInput();
    p_input->onUpdate();
    if (p_input->getRawButtons() & Input::Button::DOWN) {
        reset_usb_boot(0, 0);
        while (true) tight_loop_contents();
    }

    // overclock
#if defined(PICO_RP2350)
#if defined(GPIO_PIN_PSRAM_CS)
    // https://github.com/earlephilhower/arduino-pico/blob/master/cores/rp2040/main.cpp
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    busy_wait_at_least_cycles((SYS_CLK_VREG_VOLTAGE_AUTO_ADJUST_DELAY_US * static_cast<uint64_t>(XOSC_HZ)) / 1000000);

    constexpr uint8_t clockDiv = ((333000 * 1000) + 109000000 - 1) / 109000000;
    printf("psram_reinit_timing_custom: clock divider = %i\r\n", clockDiv);

    // need to increase the qmi divider before upping sysclk to ensure we keep the output sck w/in legal bounds
    psram_reinit_timing_custom(333000 * 1000);

    // per datasheet, need to do a dummy access and memory barrier before it takes effect
    volatile uint8_t *x = &__psram_start__;
    *x ^= 0xff;
    *x ^= 0xff;
    asm volatile("" ::: "memory");

    // set actual clock
    set_sys_clock_khz(333000, true);
    sleep_ms(10);
#else
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    busy_wait_at_least_cycles((SYS_CLK_VREG_VOLTAGE_AUTO_ADJUST_DELAY_US * static_cast<uint64_t>(XOSC_HZ)) / 1000000);
    // set actual clock
    set_sys_clock_khz(399000, true);
    sleep_ms(10);
#endif
#else
    // TODO
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    busy_wait_at_least_cycles((SYS_CLK_VREG_VOLTAGE_AUTO_ADJUST_DELAY_US * static_cast<uint64_t>(XOSC_HZ)) / 1000000);
    set_sys_clock_khz(280000, false);
    sleep_ms(2);
#endif

    // DEBUG
#if defined(PICO_DEBUG_UART) || defined(PICO_DEBUG_USB)
    // initialise USB serial connection for debugging
    stdio_init_all();
#ifdef PICO_DEBUG_USB
    // wait for usb serial
    while (!stdio_usb_connected()) { sleep_ms(100); }
#endif
#endif

    printf("\r\nPicoPlatform: %s @ %lu MHz\r\n", PICO_BOARD, clock_get_hz(clk_sys) / 1000000);

#ifdef PICO_PSRAM_RP2040
    PSram::init();
#elif GPIO_PIN_PSRAM_CS
    printf("PicoPlatform: psram size: %i, used: %i\r\n", __psram_size, __psram_total_used());
#endif

    Io::init();
    p_battery = new PicoBattery();
    p_audio = new PicoAudio();
    if (displaySettings.bufferingMode == Display::Buffering::None) {
        p_display = new PicoDisplayDirectDraw(displaySettings);
    } else {
        p_display = new PicoDisplay(displaySettings);
    }
}

void PicoPlatform::reboot(uint32_t watchdog_scratch) {
    printf("PicoPlatform::reboot\r\n");

    if (p_display) {
        // clean both screen buffers
        p_display->clear();
        p_display->flip();
        p_display->clear();
        p_display->flip();
    }

    watchdog_hw->scratch[0] = watchdog_scratch;
    watchdog_reboot(0, 0, 50);

    // wait for the reset
    while (true) tight_loop_contents();
}

void PicoPlatform::sleep(const uint32_t ms) {
    sleep_ms(ms);
}

PicoPlatform::~PicoPlatform() {
    printf("~PicoPlatform()\n");
}
