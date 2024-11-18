//
// Created by cpasjuste on 29/12/23.
//

#include "platform.h"

using namespace p2d;

static Platform *s_platform = nullptr;

Platform::Platform(const Display::Settings &displaySettings) : Widget() {
    s_platform = this;
    Platform::setPosition(0, 0);
    Platform::setSize(displaySettings.displaySize.x, displaySettings.displaySize.y);
}

bool Platform::loop() {
    // input
    onInput(0);
    if (p_input->getButtons() & Input::Button::QUIT) {
        return false;
    }

    // update loop
    onUpdate(m_delta_clock.restart());

    // drawing
    onDraw(true);

    return true;
}

void Platform::onUpdate(Time delta) {
    // battery
    if (p_battery) p_battery->loop();

    // runtime
    if (m_elapsed_clock.getElapsedTime().asSeconds() == 60) {
        m_runtime_minutes++;
        m_elapsed_clock.restart();
    }

    // fps
    if (m_fps_clock.getElapsedTime().asMilliseconds() >= 1000) {
        m_fps = m_frames;
        m_frames = 0;
        m_fps_clock.restart();
        if (m_stats_print) {
            auto heapPercent = (uint16_t) (((float) Utility::getUsedHeap()
                                            / (float) Utility::getTotalHeap()) * 100);
            auto psramPercent = (uint16_t) (((float) Utility::getUsedPSRAMHeap()
                                             / (float) Utility::getTotalPSRAMHeap()) * 100);
            printf("fps: %i, heap: %i/%i (%i%%), psram: %i/%i (%i%%), battery: %i%% (%.02fv), runtime: %lu minutes\r\n",
                   m_fps,
                   Utility::getUsedHeap(), Utility::getTotalHeap(), heapPercent,
                   Utility::getUsedPSRAMHeap(), Utility::getTotalPSRAMHeap(), psramPercent,
                   p_battery->getPercent(), p_battery->getVoltage(), m_runtime_minutes);
        }
    }

    Widget::onUpdate(delta);

    m_frames++;
}

bool Platform::onInput(const uint16_t &dummy) {
    // update buttons state
    p_input->onUpdate();

    auto buttons = p_input->getButtons();
    if (buttons && buttons != Input::Button::DELAY) {
        // call input callback on childs
        return Widget::onInput(buttons);
    }

    return false;
}

void Platform::onDraw(const bool draw) {
    if (p_display && draw) {
        // clear screen
        p_display->clear();

        // draw child's
        Widget::onDraw(draw);

        // flip screen
        p_display->flip();
    }
}

Platform *Platform::instance() {
    return s_platform;
}

Platform::~Platform() {
    printf("~Platform()\n");
    delete (p_display);
    delete (p_audio);
    delete (p_input);
    delete (p_battery);
    Io::exit();
}
