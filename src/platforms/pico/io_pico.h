//
// Created by cpasjuste on 01/06/23.
//

#ifndef MICROBOY_IO_PICO_H
#define MICROBOY_IO_PICO_H

#include "sd_card.h"

// 4MB available of 8MB (Feather rp2040) (1MB for rom data, 3MB for misc data)
#define FLASH_TARGET_OFFSET_ROM_HEADER ((1024 * 1024) * 4)  // unused for now
//#define FLASH_TARGET_OFFSET_ROM_HEADER ((uint32_t) &__FLASHLOADER_END)
#define FLASH_TARGET_OFFSET_ROM_DATA (FLASH_TARGET_OFFSET_ROM_HEADER + FLASH_SECTOR_SIZE)
#define FLASH_TARGET_OFFSET_MISC (FLASH_TARGET_OFFSET_ROM_DATA + FLASH_BLOCK_SIZE + (1024 * 1024)) // 1MB max rom size


namespace p2d {
    class PicoIo : public Io {
    public:
        PicoIo();

        FileBuffer read(const std::string &path, const Target &target = Flash) override;

        FileBuffer readRomFromFlash() override;

        bool write(const std::string &path, const FileBuffer &fileBuffer) override;

        bool writeRomToFlash(const std::string &path, const std::string &name) override;

        FileListBuffer getDir(const std::string &path) override;

        void createDir(const std::string &path) override;

    private:
        sd_card_t *p_sd = nullptr;
        size_t m_flash_offset_misc = FLASH_TARGET_OFFSET_MISC;

        bool mount();

        bool unmount();

        static void writeSector(uint32_t flash_offs, const uint8_t *data);
    };
}

#endif //MICROBOY_IO_PICO_H
