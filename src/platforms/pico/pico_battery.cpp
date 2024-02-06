//
// Created by cpasjuste on 01/02/24.
//

#include <hardware/adc.h>
#include "battery.h"
#include "pico_battery.h"
#include "pinout.h"

using namespace p2d;

#define CONV_FACTOR (3.3f / (1 << 12))

PicoBattery::PicoBattery(float vMin, float vMax) : Battery(vMin, vMax) {
#if defined(BTN_PIN_VBAT)
    adc_init();
    adc_gpio_init(BTN_PIN_VBAT);
    adc_select_input(2);
#endif
}

float PicoBattery::getVoltageRaw() const {
#if defined(BTN_PIN_VBAT)
    return (float) adc_read() * 2 * CONV_FACTOR;
#else
    return 100;
#endif
}
