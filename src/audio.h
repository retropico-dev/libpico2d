//
// Created by cpasjuste on 31/05/23.
//

#ifndef MICROBOY_AUDIO_H
#define MICROBOY_AUDIO_H

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

        virtual void volumeUp() {
            if (m_volume <= m_volume_max) {
                m_volume += 10;
            }
        }

        virtual void volumeDown() {
            if (m_volume >= 10) {
                m_volume -= 10;
            }
        }

        virtual uint8_t getVolume() { return m_volume; }

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

#endif //MICROBOY_AUDIO_H
