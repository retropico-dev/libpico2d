//
// Created by cpasjuste on 30/05/23.
//

#ifdef PICO_PSRAM

#include <cstdio>
#include <hardware/clocks.h>
#include "platform.h"
#include "pico_display.h"
#include "st7789.h"

using namespace p2d;

static PicoDisplay *s_display;

static Utility::Vec2i s_display_size;
static Utility::Vec4i s_render_bounds;
static uint8_t s_bit_shift;

static pimoroni::APS6404 *p_ram;
static uint32_t s_sram_surfaces[2];

// core1 stuff
#define USE_CORE1 0
#if USE_CORE1
#define CORE1_CMD_FLIP 0
#define CORE1_CMD_EXIT 1

union core_cmd {
    struct {
        uint8_t cmd;
        uint8_t fb;
    };
    uint32_t full;
};

static core_cmd cmd_flip{.cmd = CORE1_CMD_FLIP};
static core_cmd cmd_exit{.cmd = CORE1_CMD_EXIT};
static int core1_busy = 0;
static mutex_t mutex;

static void in_ram(core1_main)();
#endif

PicoDisplay::PicoDisplay(const Utility::Vec2i &displaySize, const Utility::Vec2i &renderSize,
                         const Utility::Vec4i &renderBounds, const Buffering &buffering,
                         const Format &format, float spiSpeedMhz)
        : Display(displaySize, renderSize, renderBounds, buffering, format, spiSpeedMhz) {
    // init st7789 display
    st7789_init(m_format == Format::RGB565 ?
                ST7789_COLOR_MODE_16BIT : ST7789_COLOR_MODE_12BIT, spiSpeedMhz);
    //st7789_set_backlight(255);

    // handle alpha channel removal (st7789 support rgb444)
    if (m_format == Format::ARGB444) s_bit_shift = m_bit_shift = 4;

    // my own core1 crap
    s_display = this;
    s_display_size = m_displaySize;
    s_render_bounds = m_renderBounds;

    p_ram = PSram::getRam();
    s_sram_surfaces[0] = PSram::alloc(240 * 240 * 2, true);
    s_sram_surfaces[1] = PSram::alloc(240 * 240 * 2, true);

#if USE_CORE1
    mutex_init(&mutex);
#if defined(PICO_DEBUG_UART)
    multicore_reset_core1(); // seems to be needed for "picoprobe" debugging
#endif
    multicore_launch_core1(core1_main);
#endif
}

__always_inline void in_ram(PicoDisplay::setCursor)(int16_t x, int16_t y) {
    m_cursor = {x, y};
}

__always_inline void in_ram(PicoDisplay::setPixel)(uint16_t color) {
    // do not write outside render bounds
    if (color != m_colorKey && m_cursor.x < m_renderSize.x && m_cursor.y < m_renderSize.y) {
#if USE_CORE1
        mutex_enter_blocking(&mutex);
        p_ram->wait_for_finish_blocking();
#endif
        PSram::write(s_sram_surfaces[m_bufferIndex] + (m_cursor.y * m_pitch + m_cursor.x * m_bpp), color, 1);
#if USE_CORE1
        mutex_exit(&mutex);
#endif
    }

    // emulate tft lcd "put_pixel"
    m_cursor.x++;
    if (m_cursor.x >= m_renderSize.x) {
        m_cursor.x = 0;
        m_cursor.y += 1;
    }
}

__always_inline void in_ram(PicoDisplay::drawPixelLine)(const uint16_t *pixels, uint16_t width) {
#if USE_CORE1
    mutex_enter_blocking(&mutex);
    p_ram->wait_for_finish_blocking();
#endif
    p_ram->write(s_sram_surfaces[m_bufferIndex] + (m_cursor.y * m_pitch + m_cursor.x * m_bpp),
                 (uint32_t *) pixels, width * m_bpp);
#if USE_CORE1
    mutex_exit(&mutex);
#endif
}

__always_inline void in_ram(PicoDisplay::clear)() {
#if USE_CORE1
    mutex_enter_blocking(&mutex);
    p_ram->wait_for_finish_blocking();
#endif
    PSram::write(s_sram_surfaces[m_bufferIndex], m_clearColor, m_renderSize.x * m_renderSize.y);
#if USE_CORE1
    mutex_exit(&mutex);
#endif
}

__always_inline void in_ram(PicoDisplay::flip)() {
#if USE_CORE1
    // wait for previous buffer flip if needed
    while (__atomic_load_n(&core1_busy, __ATOMIC_SEQ_CST)) {
        //printf("PicoDisplay::flip: waiting for rendering...\r\n");
    }

    // send core1 flip "cmd"
    cmd_flip.fb = m_bufferIndex;
    __atomic_store_n(&core1_busy, 1, __ATOMIC_SEQ_CST);
    multicore_fifo_push_blocking(cmd_flip.full);
#else
#if TEST_LINE_BUFFER
    st7789_set_cursor(0, 0);

    for (int y = 0; y < m_renderSize.y; y++) {
        p_ram->read_blocking(s_sram_surfaces[m_bufferIndex] + y * m_pitch,
                             (uint32_t *) frame_buffer, m_renderSize.x / 2);
        st7789_push();
    }
#else
    PSram::flush();
    p_ram->read_blocking(s_sram_surfaces[m_bufferIndex], (uint32_t *) frame_buffer, (240 * 240) / 2);

    st7789_dma_flush();
    st7789_set_cursor(0, 0);
    // TODO: handle size
    st7789_push();
#endif
#endif

    // flip buffers
    m_bufferIndex = !m_bufferIndex;
}

#if !USE_CORE1

void in_ram(p2d_display_pause)() {}

void in_ram(p2d_display_resume)() {}

#else
// crappy hack to pause core1 while writing to flash,
// I was not able to use "multicore_lockout_victim_init" without random crashes
static bool core1_stop = false;
static bool core1_stopped = true;

void in_ram(p2d_display_pause)() {
    //printf("p2d_display_pause\r\n");

    if (!core1_stopped) {
        core1_stop = true;
        multicore_fifo_push_blocking(cmd_exit.full);
        while (!core1_stopped) tight_loop_contents();
    }

    //printf("p2d_display_pause: true\r\n");
}

void in_ram(p2d_display_resume)() {
    //printf("p2d_display_resume\r\n");

    if (core1_stop) {
        multicore_reset_core1();
        multicore_launch_core1(core1_main);
    }

    //printf("p2d_display_resume: true\r\n");
}

static void in_ram(core1_main)() {
    core_cmd cmd{};

    //multicore_lockout_victim_init();
    core1_stop = false;
    core1_stopped = false;

    while (!core1_stop) {
        cmd.full = multicore_fifo_pop_blocking();
        if (cmd.cmd == CORE1_CMD_FLIP) {
            st7789_set_cursor(0, 0);

            for (int y = 0; y < 240; y++) {
                mutex_enter_blocking(&mutex);
                p_ram->wait_for_finish_blocking();
                p_ram->read_blocking(s_sram_surfaces[cmd.fb] + y * (240 * 2), (uint32_t *) frame_buffer, 120);
                mutex_exit(&mutex);
                st7789_update();
            }
        }

        __atomic_store_n(&core1_busy, 0, __ATOMIC_SEQ_CST);
    }

    core1_stopped = true;
}
#endif

#endif