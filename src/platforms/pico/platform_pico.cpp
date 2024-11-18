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

#include <sys/unistd.h>

using namespace p2d;

PicoPlatform::PicoPlatform(const Display::Settings &displaySettings) : Platform(displaySettings) {
    // first thing to do (reboot bootloader on key down)
    p_input = new PicoInput();
    p_input->onUpdate();
    if (p_input->getRawButtons() & Input::Button::DOWN) {
        reset_usb_boot(0, 0);
        while (true) tight_loop_contents();
    }

    vreg_set_voltage(VREG_VOLTAGE_DEFAULT);
    sleep_ms(2);

    // overclock
#ifdef GPIO_PIN_PSRAM_CS
    set_sys_clock_khz(250000, false);
    sleep_ms(2);
    psram_reinit_timing();
    sleep_ms(2);
#else
    set_sys_clock_khz(280000, false);
    sleep_ms(2);
#endif

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
