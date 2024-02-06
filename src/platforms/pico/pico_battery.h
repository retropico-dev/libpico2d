//
// Created by cpasjuste on 01/02/24.
//

#ifndef PICO2D_PICO_BATTERY_H
#define PICO2D_PICO_BATTERY_H

namespace p2d {
    class PicoBattery : public Battery {
    public:
        explicit PicoBattery(float vMin = 3.0f, float vMax = 4.2f);

    private:
        [[nodiscard]] float getVoltageRaw() const override;
    };
}

#endif //PICO2D_PICO_BATTERY_H
