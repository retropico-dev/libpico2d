//
// Created by cpasjuste on 26/05/23.
//

#ifndef TAMAGOPICO_CLOCK_TIME_H
#define TAMAGOPICO_CLOCK_TIME_H

#include <cstdint>

namespace mb {
    class Time {
    public:
        Time();

        [[nodiscard]] float asSeconds() const;

        [[nodiscard]] uint32_t asMilliseconds() const;

        [[nodiscard]] uint64_t asMicroseconds() const;

    private:

        friend Time seconds(float);

        friend Time milliseconds(uint32_t);

        friend Time microseconds(uint64_t);

        explicit Time(uint64_t microseconds);

        uint64_t m_microseconds;
    };

    Time seconds(float amount);

    Time milliseconds(uint32_t amount);

    Time microseconds(uint64_t amount);

    bool operator==(Time left, Time right);

    bool operator!=(Time left, Time right);

    bool operator<(Time left, Time right);

    bool operator>(Time left, Time right);

    bool operator<=(Time left, Time right);

    bool operator>=(Time left, Time right);

    Time operator-(Time right);

    Time operator+(Time left, Time right);

    Time &operator+=(Time &left, Time right);

    Time operator-(Time left, Time right);

    Time &operator-=(Time &left, Time right);

    Time operator*(Time left, float right);

    Time operator*(Time left, long right);

    Time operator*(float left, Time right);

    Time operator*(long left, Time right);

    Time &operator*=(Time &left, float right);

    Time &operator*=(Time &left, long right);

    Time operator/(Time left, float right);

    Time operator/(Time left, long right);

    Time &operator/=(Time &left, float right);

    Time &operator/=(Time &left, long right);

    float operator/(Time left, Time right);

    Time operator%(Time left, Time right);

    Time &operator%=(Time &left, Time right);
}

#endif //TAMAGOPICO_CLOCK_TIME_H
