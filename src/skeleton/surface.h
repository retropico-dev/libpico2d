//
// Created by cpasjuste on 05/06/23.
//

#ifndef MICROBOY_SURFACE_H
#define MICROBOY_SURFACE_H

#include <cstring>
#include <cstdlib>
#include "utility.h"

namespace mb {
    class Surface {
    public:
        static Surface *fromBmp(const uint8_t *binary) {
            auto bmp = (bmp_header *) binary;
            if ((bmp->bmp_file_header[0] != 'B') || (bmp->bmp_file_header[1] != 'M')) {
                printf("Surface::fromBmp: binary is not a bitmap dump...\r\n");
                return nullptr;
            }

            if (bmp->bmp_info_header[14] != 16) {
                printf("Surface::fromBmp: bitmap bits should be 16...\r\n");
                return nullptr;
            }

            uint16_t w = (bmp->bmp_info_header[4] + (bmp->bmp_info_header[5] << 8)
                          + (bmp->bmp_info_header[6] << 16) + (bmp->bmp_info_header[7] << 24));
            uint16_t h = (bmp->bmp_info_header[8] + (bmp->bmp_info_header[9] << 8)
                          + (bmp->bmp_info_header[10] << 16) + (bmp->bmp_info_header[11] << 24));
            if (w == 0 || h == 0) {
                printf("Surface::fromBmp: bitmap size is wrong...\r\n");
                return nullptr;
            }

            printf("Surface::fromBmp: loading new bitmap (%i x %i @ %i bpp)\r\n",
                   w, h, bmp->bmp_info_header[14]);

            return new Surface({(int16_t) w, (int16_t) h}, (uint8_t *) binary + 54);
        }

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

        uint8_t *getPixels() { return p_buffer; };

        [[nodiscard]] uint32_t getPixelsSize() const { return m_size.y * m_pitch; };

        [[nodiscard]] Utility::Vec2i getSize() const { return m_size; };

        [[nodiscard]] uint16_t getPitch() const { return m_pitch; };

        [[nodiscard]] uint8_t getBpp() const { return m_bpp; };

    private:
        typedef struct bmp_header {
            uint8_t bmp_file_header[14];
            uint8_t bmp_info_header[40];
            //uint8_t bmp_pad[3];
            //uint8_t *data;
        } bmp_header;

        uint8_t *p_buffer = nullptr;
        Utility::Vec2i m_size;
        uint16_t m_pitch = 0;
        uint8_t m_bpp = 2;
        bool m_read_only = false;
    };
}

#endif //MICROBOY_SURFACE_H
