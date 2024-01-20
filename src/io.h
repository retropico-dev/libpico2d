//
// Created by cpasjuste on 01/06/23.
//

#ifndef PICO2D_IO_H
#define PICO2D_IO_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include "io_file.h"

#define IO_MAX_FILES 2048
#define IO_MAX_PATH 128

namespace p2d {
    class Io {
    public:
        enum Device {
            Sd = 0,
            Flash = 1,
            //Ram
        };

        struct BufferFile {
            const uint8_t *ptr;
            uint32_t length;
        };

        static void init();

        static void fini();

#warning "TODO: is_device_available"
        //bool is_device_available(const Device &device);

        static std::vector<File::Info> list(
                const std::string &path, std::function<bool(const File::Info &)> filter = nullptr);

        static bool copy(const File &src, const File &dst);

        static bool copy(const std::string &src, const std::string &dst);

        static bool remove(const std::string &path);

        static bool fileExists(const std::string &path);

        static bool directoryExists(const std::string &path);

        static bool create(const std::string &path);

        static bool rename(const std::string &old_name, const std::string &new_name);

        static bool format(const Device &device);

        class FatFs {
        public:
            static void *open_file(const std::string &file, int mode);

            static int32_t read_file(void *fh, uint32_t offset, uint32_t length, char *buffer);

            static int32_t write_file(void *fh, uint32_t offset, uint32_t length, const char *buffer);

            static int32_t close_file(void *fh);

            static uint32_t get_file_length(void *fh);

            static void list_files(const std::string &path, std::function<void(File::Info &)> callback);

            static bool file_exists(const std::string &path);

            static bool remove_file(const std::string &path);

            static bool is_files_open();

            static void close_open_files();
        };
    };
}

#endif //PICO2D_IO_H
