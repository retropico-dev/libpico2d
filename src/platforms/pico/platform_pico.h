//
// Created by cpasjuste on 30/05/23.
//

#ifndef MICROBOY_PLATFORM_PICO_H
#define MICROBOY_PLATFORM_PICO_H

#include <pico/stdio.h>
#include <pico/bootrom.h>
#include <pico/multicore.h>
#include <pico/stdio.h>
#include <hardware/gpio.h>
#include <hardware/sync.h>
#include <hardware/flash.h>
#include <hardware/sync.h>

#include "pico_display.h"
#include "pico_display_direct.h"
#include "input_pico.h"
#include "audio_pico.h"
#include "io_pico.h"

#define in_ram __not_in_flash_func

namespace p2d {
    class PicoPlatform : public Platform {
    public:
        explicit PicoPlatform(bool overclock = false);

        ~PicoPlatform() override;

        void reboot(uint32_t watchdog_scratch = 0) override;
    };
}

#endif //MICROBOY_PLATFORM_PICO_H
