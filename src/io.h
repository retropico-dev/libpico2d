//
// Created by cpasjuste on 01/06/23.
//

#ifndef PICO2D_IO_H
#define PICO2D_IO_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include "utility.h"
#include "flash.h"
#include "sdcard.h"
#include "romfs.h"

// 16MB flash: 6MB for bootloader/apps/cache, 10MB for fatfs "user" data/cache
#define FLASH_TARGET_OFFSET_CACHE ((1024 * 1024) * 5)   // 1MB flash cache (raw)
#define FLASH_TARGET_OFFSET_FATFS ((1024 * 1024) * 6)   // 10MB fatfs flash ("flash:")

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

        struct FileBuffer {
            const uint8_t *ptr{};
            uint32_t length{};
        };

        struct ListBuffer {
            uint8_t *data = nullptr;
            uint32_t data_size = 0;
            uint16_t count = 0;

            [[nodiscard]] char *at(int idx) const {
                return (char *) &data[idx * IO_MAX_PATH];
            }
        };

        class File final {
        public:
            enum OpenMode {
                Read = 1 << 0,
                Write = 1 << 1
            };

            enum Flags {
                Directory = 1
            };

            struct Info {
                std::string name;
                int flags;
                uint32_t size;
            };

            File() = default;

            explicit File(const std::string &path, int mode = OpenMode::Read) {
                m_path = path;
                m_name = Utility::baseName(m_path);
                open(path, mode);
            }

            File(const std::string &path, const uint8_t *buf, uint32_t buf_len) {
                m_path = path;
                m_name = Utility::baseName(m_path);
                open(buf, buf_len);
            }

            File(const File &) = delete;

            File(File &&other) noexcept { *this = std::move(other); }

            ~File() { close(); }

            File &operator=(const File &) = delete;

            File &operator=(File &&other) noexcept {
                if (this != &other) {
                    close();
                    std::swap(m_path, other.m_path);
                    std::swap(p_fh, other.p_fh);
                    std::swap(buf, other.buf);
                    std::swap(buf_len, other.buf_len);
                }
                return *this;
            }

            bool open(const std::string &file, int mode = OpenMode::Read);

            bool open(const uint8_t *buf, uint32_t buf_len);

            int32_t read(uint32_t offset, uint32_t length, char *buffer) const;

            int32_t write(uint32_t offset, uint32_t length, const char *buffer) const;

            void close();

            [[nodiscard]] uint32_t getLength() const;

            [[nodiscard]] bool isOpen() const { return buf != nullptr || p_fh != nullptr; }

            [[nodiscard]] const uint8_t *getPtr() const { return buf; };

            [[nodiscard]] std::string getPath() const { return m_path; }

            [[nodiscard]] std::string getName() const { return m_name; }

            [[nodiscard]] void *getHandler() const { return p_fh; }

            static void addBufferFile(const std::string &path, const uint8_t *ptr, uint32_t len);

        private:
            std::string m_path;
            std::string m_name;
            void *p_fh = nullptr;
            const uint8_t *buf = nullptr;
            uint32_t buf_len{};
        };

        static void init();

        static void exit();

        static bool isAvailable(const Device &device);

        static bool create(const std::string &path);

        static bool remove(const std::string &path);

        static bool rename(const std::string &old_name, const std::string &new_name);

        static bool fileExists(const std::string &path);

        static bool directoryExists(const std::string &path);

        static bool copy(const File &src, const File &dst);

        static bool copy(const std::string &src, const std::string &dst);

        static std::vector<File::Info> getList(
                const std::string &path, std::function<bool(const File::Info &)> const &filter = nullptr);

        static ListBuffer getBufferedList(const std::string &path, uint32_t flashOffset);

        static bool format(const Device &device);

        class FatFs {
        public:
            static void *open_file(const std::string &path, int mode);

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
