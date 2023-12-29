//
// Created by cpasjuste on 30/05/23.
//

#include <pico/time.h>
#include <pico/stdlib.h>
#include <hardware/vreg.h>
#include <hardware/watchdog.h>
#include "platform.h"
#include "platform_pico.h"

using namespace mb;

PicoPlatform::PicoPlatform(bool useDoubleBufferDisplay, bool maxOc) : Platform() {
    // overclock
    if (maxOc) {
        vreg_set_voltage(VREG_VOLTAGE_1_15);
        sleep_ms(2);
        set_sys_clock_khz(300000, true);
        sleep_ms(2);
    } else {
        const unsigned vco = 1596 * 1000 * 1000; // 266MHz
        const unsigned div1 = 6, div2 = 1;
        vreg_set_voltage(VREG_VOLTAGE_DEFAULT);
        sleep_ms(2);
        set_sys_clock_pll(vco, div1, div2);
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
    printf("\r\nPicoPlatform: pico\r\n");

    p_display = useDoubleBufferDisplay ? (PicoDisplay *) new PicoDisplayBuffered() : new PicoDisplay();
    p_audio = new PicoAudio();
    p_input = new PicoInput();
    p_io = new PicoIo();
}

void PicoPlatform::reboot() {
    printf("PicoPlatform::reboot\r\n");

    // clean screen
    p_display->clear();
    p_display->flip();

    watchdog_reboot(0, 0, 50);

    // wait for the reset
    while (true) tight_loop_contents();
}

PicoPlatform::~PicoPlatform() {
    printf("~PicoPlatform()\n");
}
