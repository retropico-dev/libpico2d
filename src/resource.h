//
// Created by cpasjuste on 06/01/24.
//

#ifndef PICO2D_RESOURCE_H
#define PICO2D_RESOURCE_H

#include <cstddef>

namespace p2d {
    class Resource {
    public:
        Resource(const uint8_t *start, const uint8_t *end) : mData(start), mSize(end - start) {}

        [[nodiscard]] const uint8_t *const &data() const { return mData; }

        [[nodiscard]] const size_t &size() const { return mSize; }

        [[nodiscard]] const uint8_t *begin() const { return mData; }

        [[nodiscard]] const uint8_t *end() const { return mData + mSize; }

    private:
        const uint8_t *mData;
        size_t mSize;
    };
}

#define P2D_GET_RES(x) ([]() {                                    \
        extern const uint8_t _binary_##x##_start, _binary_##x##_end;   \
        return Resource(&_binary_##x##_start, &_binary_##x##_end);  \
    })()

#endif //PICO2D_RESOURCE_H
