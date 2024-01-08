//
// Created by cpasjuste on 31/05/23.
//

#include "platform.h"
#include "audio_linux.h"

using namespace mb;

LinuxAudio::LinuxAudio() {
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        SDL_InitSubSystem(SDL_INIT_AUDIO);
    }
}

void LinuxAudio::setup(uint16_t rate, uint16_t samples, uint8_t channels) {
    // call base class
    Audio::setup(rate, samples, channels);

    // first close device if already setup
    if (m_dev != 0u) {
        SDL_PauseAudioDevice(m_dev, 1);
        SDL_CloseAudioDevice(m_dev);
    }

    SDL_AudioSpec want, got;
    want.freq = rate;
    want.format = AUDIO_S16;
    want.channels = m_channels;
    want.samples = samples;
    want.callback = nullptr;
    want.userdata = nullptr;

    if ((m_dev = SDL_OpenAudioDevice(nullptr, 0, &want, &got, 0)) == 0) {
        printf("LinuxAudio::setup:  could not open audio device: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    printf("LinuxAudio::setup: \n\tfreq: %i\n\tsamples: %i\n\tdriver: %s\n",
           got.freq, got.samples,
           SDL_GetAudioDeviceName(0, 0));

    SDL_PauseAudioDevice(m_dev, 0);
}

void LinuxAudio::play(const void *data, int samples) {
    if (!m_dev) return;
    SDL_QueueAudio(m_dev, data, samples * m_channels * sizeof(int16_t));
}

LinuxAudio::~LinuxAudio() {
    if (m_dev != 0) {
        SDL_PauseAudioDevice(m_dev, 1);
        SDL_CloseAudioDevice(m_dev);
    }

    if (SDL_WasInit(SDL_INIT_AUDIO)) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
}
