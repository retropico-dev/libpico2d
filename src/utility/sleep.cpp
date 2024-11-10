//
// Created by cpasjuste on 26/05/23.
//

#ifdef PICO_RP2350
#undef PICO_BUILD
#endif

#include <cstdio>

#ifdef PICO_BUILD

#include <pico/stdlib.h>
#include <pico/sleep.h>
#include <hardware/rosc.h>
#include <hardware/clocks.h>
#include <pico/runtime_init.h>
#include "pinout.h"

#endif

#include "sleep.h"

using namespace p2d;

#if 0
static bool awake;

static void sleep_callback() {
    awake = true;
}

static void rtc_sleep() {
    awake = false;

#ifndef LINUX
    // Start on Friday 5th of June 2020 15:45:00
    datetime_t t = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 00
    };

    // Alarm 10 seconds later
    datetime_t t_alarm = {
            .year  = 2020,
            .month = 06,
            .day   = 05,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 15,
            .min   = 45,
            .sec   = 5
    };

    // Start the RTC
    rtc_init();
    rtc_set_datetime(&t);

    printf("sleeping for 5 seconds\n");
    uart_default_tx_wait_blocking();
    sleep_goto_sleep_until(&t_alarm, &sleep_callback);
#endif
}
#endif

#ifdef PICO_BUILD

static void rosc_reset() {
    uint32_t tmp = rosc_hw->ctrl;
    tmp &= (~ROSC_CTRL_ENABLE_BITS);
    tmp |= (ROSC_CTRL_ENABLE_VALUE_ENABLE << ROSC_CTRL_ENABLE_LSB);
    rosc_write(&rosc_hw->ctrl, tmp);
    // Wait for stable
    while ((rosc_hw->status & ROSC_STATUS_STABLE_BITS) != ROSC_STATUS_STABLE_BITS);
}

#endif

void Sleep::sleep() {
#if defined(PICO_BUILD) && defined(BTN_PIN_SLEEP) && BTN_PIN_SLEEP > -1
    // be sure sleep button is released
    while (!gpio_get(BTN_PIN_SLEEP)) tight_loop_contents();

    printf("Sleep::sleep: entering dormant mode\r\n");
    uart_default_tx_wait_blocking();

    // save current sys clock
    uint32_t saved_clock = clock_get_hz(clk_sys);

    // power-off display if possible
#ifdef LCD_PIN_BL
    gpio_put(LCD_PIN_BL, false);
    sleep_ms(5);
#endif

    // go to dormant mode
    sleep_run_from_xosc();
    sleep_goto_dormant_until_edge_high(BTN_PIN_SLEEP);

    // reset clocks
    rosc_reset();
    clocks_init();

    // reset overclocking
    sleep_ms(2);
    set_sys_clock_khz(saved_clock / 1000, false);
    sleep_ms(2);

    stdio_init_all();
    uart_default_tx_wait_blocking();

    // be sure sleep button is released
    while (!gpio_get(BTN_PIN_SLEEP)) tight_loop_contents();

    printf("Sleep::sleep: exiting dormant mode, clock restored @ %i Mhz\r\n",
           (uint16_t) (clock_get_hz(clk_sys) / 1000000));

#ifdef LCD_PIN_BL
    gpio_put(LCD_PIN_BL, true);
    sleep_ms(5);
#endif
#else
    //printf("Sleep::sleep: not enabled...\n");
#endif
}

#ifdef PICO_RP2350
#define PICO_BUILD
#endif
