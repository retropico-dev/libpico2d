//
// Created by cpasjuste on 31/05/23.
//

#ifndef P2D_AUDIO_LINUX_H
#define P2D_AUDIO_LINUX_H

namespace p2d {
    class LinuxAudio : public Audio {
    public:
        explicit LinuxAudio();

        ~LinuxAudio() override;

        void setup(uint16_t rate, uint16_t samples, uint8_t channels = 2) override;

        void play(const void *data, int samples) override;

    private:
        SDL_AudioDeviceID m_dev = 0;
    };
}

#endif //P2D_AUDIO_LINUX_H
