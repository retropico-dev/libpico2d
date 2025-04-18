//
// Created by cpasjuste on 30/05/23.
//

#ifndef P2D_UTILITY_H
#define P2D_UTILITY_H

#include <cstdint>
#include <string>
#include <malloc.h>
#include "clock.h"

extern "C" char __StackLimit;
extern "C" char __bss_end__;

#ifdef GPIO_PIN_PSRAM_CS
#include "psram.h"
#endif

namespace p2d {
    class Utility {
    public:
        struct Vec2i {
            int16_t x;
            int16_t y;

            Vec2i() : x(0), y(0) {
            }

            Vec2i(const int16_t x, const int16_t y) : x(x), y(y) {
            }

            bool operator==(const Vec2i &rhs) const {
                return rhs.x == x && rhs.y == y;
            }

            bool operator!=(const Vec2i &rhs) const {
                return rhs.x != x || rhs.y != y;
            }

            Vec2i operator+(const Vec2i &rhs) const {
                return {(int16_t) (x + rhs.x), (int16_t) (y + rhs.y)};
            }

            Vec2i operator-(const Vec2i &rhs) const {
                return {(int16_t) (x - rhs.x), (int16_t) (y - rhs.y)};
            }

            Vec2i operator*(const int16_t i) const {
                return {(int16_t) (x * i), (int16_t) (y * i)};
            }
        };

        struct Vec4i {
            int16_t x;
            int16_t y;
            int16_t w;
            int16_t h;

            Vec4i() : x(0), y(0), w(0), h(0) {
            }

            Vec4i(const int16_t x, const int16_t y, const int16_t w, const int16_t h)
                : x(x), y(y), w(w), h(h) {
            }

            Vec4i operator+(const Vec4i &rhs) const {
                return {
                    (int16_t) (x + rhs.x), (int16_t) (y + rhs.y),
                    (int16_t) (w + rhs.w), (int16_t) (h + rhs.h)
                };
            }

            Vec4i operator-(const Vec4i &rhs) const {
                return {
                    (int16_t) (x - rhs.x), (int16_t) (y - rhs.y),
                    (int16_t) (w - rhs.w), (int16_t) (h - rhs.h)
                };
            }

            Vec4i operator*(const int16_t i) const {
                return {
                    (int16_t) (x * i), (int16_t) (y * i),
                    (int16_t) (w * i), (int16_t) (h * i)
                };
            }
        };

        static std::string replace(const std::string &str, const std::string &from, const std::string &to);

        static std::string remove(const std::string &str, const std::string &sub);

        static std::string removeExt(const std::string &str);

        static std::string baseName(const std::string &path);

        static bool startWith(const std::string &str, const std::string &value, bool sensitive = true);

        static bool endsWith(const std::string &value, const std::string &ending, bool sensitive = true);

        static std::string toLower(const std::string &str);

        static std::string toUpper(const std::string &str);

        static inline int getHeapSize() {
#ifdef LINUX
            return 0;
#else
            return __StackLimit;
#endif
        }

        static inline int getTotalHeap() {
#ifdef LINUX
            return 0;
#else
            return &__StackLimit - &__bss_end__;
#endif
        }

        static inline int getUsedHeap() {
#ifdef LINUX
            return 0;
#else
            struct mallinfo m = mallinfo();
            return m.uordblks;
#endif
        }

        static inline int getFreeHeap() {
            return getTotalHeap() - getUsedHeap();
        }

        static inline int getPSRAMSize() {
#if defined(GPIO_PIN_PSRAM_CS)

            return __psram_size;
#else
            return 0;
#endif
        }

        static inline int getUsedPSRAMHeap() {
#if defined(GPIO_PIN_PSRAM_CS)
            return __psram_total_used();
#else
            return 0;
#endif
        }

        static inline int getTotalPSRAMHeap() {
#if defined(GPIO_PIN_PSRAM_CS)
            return __psram_total_space();
#else
            return 0;
#endif
        }

        static inline int getFreePSRAMHeap() {
            return getTotalPSRAMHeap() - getUsedPSRAMHeap();
        }

        static std::string parseSize(uint64_t size);

        static void timerStart();

        static uint64_t timerStopPrintMicro();

        static uint32_t timerStopPrintMillis();
    };
}

#endif //P2D_UTILITY_H
