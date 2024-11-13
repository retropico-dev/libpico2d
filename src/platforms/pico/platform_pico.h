//
// Created by cpasjuste on 30/05/23.
//

#ifndef P2D_PLATFORM_PICO_H
#define P2D_PLATFORM_PICO_H

#include <pico.h>
#include "pico_display.h"
#include "pico_display_direct.h"
#include "input_pico.h"
#include "audio_pico.h"
#include "pico_battery.h"

#ifdef GPIO_PIN_PSRAM_CS
#include "psram.h"
#endif

#define in_ram __not_in_flash_func

namespace p2d {
    class PicoPlatform : public Platform {
    public:
        explicit PicoPlatform(const Display::Settings &displaySettings);

        ~PicoPlatform() override;

        void reboot(uint32_t watchdog_scratch = 0) override;
    };
}

#endif //P2D_PLATFORM_PICO_H
