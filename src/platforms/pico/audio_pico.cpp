//
// Created by cpasjuste on 31/05/23.
//

#include <hardware/pio.h>
#include <hardware/dma.h>
#include "platform.h"
#include "pinout.h"

//#define audio_pio __CONCAT(pio, PICO_AUDIO_I2S_PIO)
#define audio_pio AUDIO_PIO
#define audio_sm AUDIO_SM

using namespace p2d;

static uint32_t dummy_frame_start;

void PicoAudio::setup(uint16_t rate, uint16_t samples, uint8_t channels) {
#if AUDIO_PIN_DATA < 0 || AUDIO_PIN_CLOCK < 0 || AUDIO_PIN_RESET < 0
    Audio::setup(rate, samples, channels);
    printf("PicoAudio::setup(DUMMY): rate: %i, samples: %i, channels: %i\r\n",
           m_rate, m_samples, m_channels);
#else
    Audio::setup(rate, samples, channels);
    printf("PicoAudio::setup: rate: %i, samples: %i, channels: %i\r\n",
           m_rate, m_samples, m_channels);

    static audio_format_t audio_format = {
            .sample_freq = rate,
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .channel_count = m_channels,
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = (uint16_t) (m_channels * 2)
    };

    // audio_i2s_setup claims
    //uint8_t pio_sm = pio_claim_unused_sm(audio_pio, true);
    uint8_t dma_channel = dma_claim_unused_channel(true);
    // audio_i2s_setup will use pio_sm_claim / dma_channel_claim so free them before
    dma_channel_unclaim(dma_channel);
    //pio_sm_unclaim(audio_pio, pio_sm);

    m_i2s_config = {
            .data_pin = AUDIO_PIN_DATA,
            .clock_pin_base = AUDIO_PIN_CLOCK,
            .dma_channel = dma_channel,
            .pio_sm = audio_sm
    };

    p_producer_pool = audio_new_producer_pool(&producer_format, 4, m_samples);
    const struct audio_format *output_format = audio_i2s_setup(&audio_format, &m_i2s_config);
    if (!output_format) {
        panic("PicoAudio::setup: audio_i2s_setup failed\n");
    }

    bool ret = audio_i2s_connect(p_producer_pool);
    if (!ret) {
        panic("PicoAudio::setup: audio_i2s_connect_extra failed\r\n");
    }

    audio_i2s_set_enabled(true);

    printf("\r\n");
#endif
}

void in_ram(PicoAudio::play)(const void *data, int samples) {
#if AUDIO_PIN_DATA < 0 || AUDIO_PIN_CLOCK < 0 || AUDIO_PIN_RESET < 0
    // sync on audio
    const uint32_t frame_time = time_us_32() - dummy_frame_start;
    // target frame time
    uint32_t target_frame_time = (samples * 1000000) / (m_rate);
    // if we completed the frame too quickly, sleep for the remaining time
    if (frame_time < target_frame_time) {
        sleep_us(target_frame_time - frame_time);
    }
    //
    dummy_frame_start = time_us_32();
#else
    auto buffer = take_audio_buffer(p_producer_pool, true);
    auto sampleBuffer = (int16_t *) buffer->buffer->bytes;
    if (m_volume == 100) {
        memcpy(sampleBuffer, data, samples * sizeof(int16_t) * m_channels);
    } else {
        auto dataBuffer = (int16_t *) data;
        for (uint_fast16_t i = 0; i < (uint_fast16_t) samples * m_channels; i++) {
            sampleBuffer[i] = (int16_t) ((dataBuffer[i] * m_volume) / 100);
        }
    }

    buffer->sample_count = samples;
    give_audio_buffer(p_producer_pool, buffer);
#endif
}
