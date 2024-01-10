//
// Created by cpasjuste on 30/05/23.
//

#include <pico/time.h>
#include <pico/stdlib.h>
#include <hardware/vreg.h>
#include <hardware/watchdog.h>
#include "platform.h"
#include "platform_pico.h"

using namespace p2d;

PicoPlatform::PicoPlatform(bool overclock) : Platform() {
    // overclock
    if (overclock) {
        vreg_set_voltage(VREG_VOLTAGE_1_25);
        sleep_ms(2);
        set_sys_clock_khz(300000, true);
        sleep_ms(2);
    } else {
        vreg_set_voltage(VREG_VOLTAGE_DEFAULT);
        sleep_ms(2);
        set_sys_clock_khz(266000, true);
        sleep_ms(2);
    }

    // initialise USB serial connection for debugging
    stdio_init_all();
#ifndef NDEBUG
#ifndef LIB_PICO_STDIO_UART
    // wait for usb serial
    while (!stdio_usb_connected()) { sleep_ms(100); }
#endif
#endif
    if (overclock) printf("\r\nPicoPlatform: %s @ 300 MHz\r\n", PICO_BOARD);
    else printf("\r\nPicoPlatform: %s @ 266 MHz\r\n", PICO_BOARD);

    p_audio = new PicoAudio();
    p_input = new PicoInput();
    p_io = new PicoIo();
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

PicoPlatform::~PicoPlatform() {
    printf("~PicoPlatform()\n");
}
