//
// Created by cpasjuste on 01/02/24.
//

#ifndef PICO2D_PICO_BATTERY_H
#define PICO2D_PICO_BATTERY_H

namespace p2d {
    class PicoBattery : public Battery {
    public:
        explicit PicoBattery(float vMin = 2.5f, float vMax = 3.7f);

    private:
        [[nodiscard]] float getVoltageRaw() const override;
    };
}

#endif //PICO2D_PICO_BATTERY_H
