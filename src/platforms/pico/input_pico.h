//
// Created by cpasjuste on 31/05/23.
//

#ifndef P2D_INPUT_PICO_H
#define P2D_INPUT_PICO_H

namespace p2d {
    class PicoInput : public Input {
    public:
        PicoInput();

        void onUpdate() override;
    };
}

#endif //P2D_INPUT_PICO_H
