//
// Created by cpasjuste on 01/02/24.
//

#ifndef PICO2D_BATTERY_H
#define PICO2D_BATTERY_H

#include <cstdint>
#include <cmath>
#include "clock.h"

namespace p2d {
    class Battery {
    public:
        explicit Battery(float vMin = 3.0f, float vMax = 4.2f) {
            m_v_max = vMax;
            m_v_min = vMin;
            m_v_range = vMax - vMin;

        };

        void loop() {
            if (m_clock.getElapsedTime().asMilliseconds() >= 500) {
                m_voltage_average += getVoltageRaw();
                m_get_voltage_count++;
            }

            if (m_clock.getElapsedTime().asMilliseconds() >= 5000) {
                m_voltage = m_voltage_average / (float) m_get_voltage_count;
                double v_now = std::fmin(m_voltage - m_v_min, m_v_range);
                m_percent = (uint8_t) std::fmax((v_now * 100 / m_v_range), 0);
                m_get_voltage_count = 0;
                m_voltage_average = 0;
                m_clock.restart();
            }
        }

        [[nodiscard]] float getVoltage() const { return m_voltage; }

        [[nodiscard]] uint8_t getPercent() const { return m_percent; }

    private:
        float m_v_min = 0;
        float m_v_max = 0;
        float m_v_range = 0;
        float m_voltage = 0;
        float m_voltage_average = 0;
        uint32_t m_get_voltage_count = 0;
        uint32_t m_percent = 0;
        Clock m_clock{};

        [[nodiscard]] virtual float getVoltageRaw() const { return 0; }
    };
}

#endif //PICO2D_BATTERY_H
