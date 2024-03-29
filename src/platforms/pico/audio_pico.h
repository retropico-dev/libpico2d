//
// Created by cpasjuste on 31/05/23.
//

#ifndef P2D_AUDIO_PICO_H
#define P2D_AUDIO_PICO_H

#include <pico/audio_i2s.h>

namespace p2d {
    class PicoAudio : public Audio {
    public:
        void setup(uint16_t rate, uint16_t samples, uint8_t channels = 2) override;

        void play(const void *data, int samples) override;

    private:
        audio_i2s_config_t m_i2s_config{};
        audio_buffer_pool *p_producer_pool = nullptr;
    };
}

#endif //P2D_AUDIO_PICO_H
