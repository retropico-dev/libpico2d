//
// Created by cpasjuste on 05/06/23.
//

#ifndef MICROBOY_SURFACE_H
#define MICROBOY_SURFACE_H

#include <cstring>
#include <cstdlib>
#include "romfs/romfs.hpp"
#include "utility.h"

namespace p2d {
    class Surface {
    public:
        Surface(const Utility::Vec2i &size, uint8_t *buffer) {
            m_size = size;
            p_buffer = buffer;
            m_pitch = size.x * m_bpp;
            m_read_only = true;
        }

        explicit Surface(const Utility::Vec2i &size, uint32_t bufferSize = 0) {
            m_size = size;
            if (bufferSize > 0) {
                p_buffer = (uint8_t *) malloc(bufferSize);
                memset(p_buffer, 0, bufferSize);
            } else {
                p_buffer = (uint8_t *) malloc(m_size.x * m_size.y * m_bpp);
                memset(p_buffer, 0, m_size.x * m_size.y * m_bpp);
            }
            m_pitch = m_size.x * m_bpp;
        }

        explicit Surface(const romfs::Resource &resource) {
            auto bmp = (BMPHeader *) resource.data();
            if ((bmp->header[0] != 'B') || (bmp->header[1] != 'M')) {
                printf("Surface: binary is not a bitmap dump...\r\n");
                return;
            }

            if (bmp->bpp != 16) {
                printf("Surface(): bitmap bits should be 16...\r\n");
                return;
            }

            if (bmp->w == 0 || bmp->h == 0) {
                printf("Surface(): bitmap size is wrong...\r\n");
                return;
            }

            printf("Surface(): loading new bitmap (%i x %i @ %i bpp)\r\n",
                   bmp->w, bmp->h, bmp->bpp);

            auto padding = ((4 - (bmp->w * 3) % 4) % 4);
            p_buffer = (uint8_t *) resource.data() + bmp->data_offset;
            m_size = {(int16_t) bmp->w, (int16_t) bmp->h};
            m_pitch = (m_size.x * m_bpp) - padding;
            m_read_only = true;
            m_is_bitmap = true;
        }

        ~Surface() {
            if (!m_read_only && p_buffer) free(p_buffer);
        }

        void setPixel(uint16_t x, uint16_t y, uint16_t pixel) {
            if (m_read_only || !p_buffer) return;
            if (x >= m_size.x || y >= m_size.y) return;
            *(uint16_t *) (p_buffer + y * m_pitch + x * m_bpp) = pixel;
        }

        void setPixel(const Utility::Vec2i &pos, uint16_t pixel) {
            setPixel(pos.x, pos.y, pixel);
        }

        uint16_t getPixel(uint16_t x, uint16_t y) {
            if (!p_buffer) return 0;
            if (x >= m_size.x || y >= m_size.y) return 0;
            return *(uint16_t *) (p_buffer + y * m_pitch + x * m_bpp);
        }

        uint16_t getPixel(const Utility::Vec2i &pos) {
            return getPixel(pos.x, pos.y);
        }

        uint8_t *getPixels() { return p_buffer; }

        [[nodiscard]] uint32_t getPixelsSize() const { return m_size.y * m_pitch; }

        [[nodiscard]] Utility::Vec2i getSize() const { return m_size; }

        [[nodiscard]] uint16_t getPitch() const { return m_pitch; }

        [[nodiscard]] uint8_t getBpp() const { return m_bpp; }

        [[nodiscard]] bool isBitmap() const { return m_is_bitmap; }

    private:
#pragma pack(push, 2)
        struct BMPHeader {
            char header[2]{};
            uint32_t file_size{};
            uint16_t reserved[2]{};
            uint32_t data_offset{};
            uint32_t info_size{};
            int32_t w{};
            int32_t h{};
            uint16_t planes{};
            uint16_t bpp{};
            uint32_t compression{};
            uint32_t image_size{};
            int32_t res_x{};
            int32_t res_y{};
            uint32_t palette_cols{};
            uint32_t important_cols{};
        };
#pragma pack(pop)

        uint8_t *p_buffer = nullptr;
        Utility::Vec2i m_size;
        uint16_t m_pitch = 0;
        uint8_t m_bpp = 2;
        bool m_read_only = false;
        bool m_is_bitmap = false;
    };
}

#endif //MICROBOY_SURFACE_H
