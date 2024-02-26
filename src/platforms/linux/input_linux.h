//
// Created by cpasjuste on 31/05/23.
//

#ifndef P2D_INPUT_LINUX_H
#define P2D_INPUT_LINUX_H

namespace p2d {
    class LinuxInput : public Input {
    public:
        void onUpdate() override;
    };
}

#endif //P2D_INPUT_LINUX_H
