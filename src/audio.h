//
// Created by cpasjuste on 31/05/23.
//

#ifndef P2D_AUDIO_H
#define P2D_AUDIO_H

#include <cstdint>

namespace p2d {
    class Audio {
    public:
        explicit Audio() = default;

        virtual ~Audio() = default;

        virtual void setup(uint16_t rate, uint16_t samples, uint8_t channels = 2) {
            m_rate = rate;
            m_samples = samples;
            m_channels = channels;
        }

        virtual void play(const void *data, int samples) {};

        // range: 0 - 100
        virtual uint8_t getVolume() { return m_volume; }

        // range: 0 - 100
        virtual void setVolume(uint8_t volume) {
            if (volume > m_volume_max) volume = m_volume_max;
            m_volume = volume;
        }

        // range: 0 - x
        virtual uint8_t getVolumeMax() { return m_volume_max; }

    protected:
        uint16_t m_rate = 44100;
        uint16_t m_samples = 735;
        uint8_t m_channels = 2;
        uint8_t m_volume = 50;
        uint8_t m_volume_max = 80;
        // max98357a + 8ohms 2w speaker, don't push too much...
        // https://aliexpress.com/item/1005005376301547.html
    };
}

#endif //P2D_AUDIO_H
