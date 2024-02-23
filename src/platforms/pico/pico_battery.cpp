//
// Created by cpasjuste on 01/02/24.
//

#include <boards/pico.h>
#include <hardware/adc.h>
#include <cstdio>
#include "battery.h"
#include "pico_battery.h"

using namespace p2d;

#define PICO_FIRST_ADC_PIN 26
#define CONV_FACTOR (3.3f * 3.0f / (1 << 12))

static bool is_usb_powered() {
#if defined PICO_VBUS_PIN
    gpio_set_function(PICO_VBUS_PIN, GPIO_FUNC_SIO);
    return !gpio_get(PICO_VBUS_PIN);
#else
    return false;
#endif
}

PicoBattery::PicoBattery(float vMin, float vMax) : Battery(vMin, vMax) {
    // init internal pin 29 (vsys adc)
    adc_init();
    adc_gpio_init(PICO_VSYS_PIN);
    adc_select_input(PICO_VSYS_PIN - PICO_FIRST_ADC_PIN);
    // init internal pin 24 (powered via usb)
    //gpio_init(24);
    //gpio_set_dir(24, GPIO_IN);
}

float PicoBattery::getVoltageRaw() const {
    //printf("getVoltageRaw: usb == %i\r\n", is_usb_powered());
    return (float) adc_read() * CONV_FACTOR;
}
