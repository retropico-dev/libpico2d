//
// Created by cpasjuste on 01/06/23.
//

#ifndef PICO2D_IO_PICO_H
#define PICO2D_IO_PICO_H

#include "flash.h"

namespace p2d {
    class PicoIo : public Io {
    public:
        PicoIo();

        ~PicoIo();

        bool directoryExists(const std::string &path) override;

        bool create(const std::string &path) override;

        bool rename(const std::string &old_name, const std::string &new_name) override;

        bool format(const Device &device) override;

        bool is_files_open_priv() override;

    protected:
        void *open_file_priv(const std::string &file, int mode) override;

        int32_t read_file_priv(void *fh, uint32_t offset, uint32_t length, char *buffer) override;

        int32_t write_file_priv(void *fh, uint32_t offset, uint32_t length, const char *buffer) override;

        int32_t close_file_priv(void *fh) override;

        uint32_t get_file_length_priv(void *fh) override;

        void list_files_priv(const std::string &path, std::function<void(FileInfo & )> callback) override;

        bool file_exists_priv(const std::string &path) override;

        bool remove_file_priv(const std::string &path) override;


        void close_open_files_priv() override;
    };
}

#endif //PICO2D_IO_PICO_H
