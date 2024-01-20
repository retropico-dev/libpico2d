//
// Created by cpasjuste on 20/01/24.
//

#ifndef PICO2D_SKELETON_IO_FILE_H
#define PICO2D_SKELETON_IO_FILE_H

#include <cstdint>
#include <string>
#include "utility.h"

namespace p2d {
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

        File(File &&other) noexcept {
            *this = std::move(other);
        }

        ~File() {
            close();
        }

        File &operator=(const File &) = delete;

        File &operator=(File &&other) noexcept {
            if (this != &other) {
                close();
                std::swap(m_path, other.m_path);
                std::swap(fh, other.fh);
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

        [[nodiscard]] uint32_t length() const;

        [[nodiscard]] bool isOpen() const { return buf != nullptr || fh != nullptr; }

        [[nodiscard]] const uint8_t *ptr() const { return buf; }

        [[nodiscard]] std::string getPath() const { return m_path; }

        [[nodiscard]] std::string getName() const { return m_name; }

        static void addBufferFile(const std::string &path, const uint8_t *ptr, uint32_t len);

    private:
        std::string m_path;
        std::string m_name;
        void *fh = nullptr;
        // buffer "files"
        const uint8_t *buf = nullptr;
        uint32_t buf_len{};
    };
}

#endif //PICO2D_SKELETON_IO_FILE_H
