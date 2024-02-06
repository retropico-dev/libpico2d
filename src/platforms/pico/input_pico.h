//
// Created by cpasjuste on 31/05/23.
//

#ifndef MICROBOY_INPUT_PICO_H
#define MICROBOY_INPUT_PICO_H

namespace p2d {
    class PicoInput : public Input {
    public:
        PicoInput();

        void onUpdate() override;
    };
}

#endif //MICROBOY_INPUT_PICO_H
