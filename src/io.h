//
// Created by cpasjuste on 01/06/23.
//

#ifndef PICO2D_IO_H
#define PICO2D_IO_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

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

        enum OpenMode {
            Read = 1 << 0,
            Write = 1 << 1
        };

        enum FileFlags {
            Directory = 1
        };

        struct FileInfo {
            std::string name;
            int flags;
            uint32_t size;
        };

        class File final {
        public:
            File() = default;

            explicit File(const std::string &filename, int mode = OpenMode::Read) { open(filename, mode); }

            File(const uint8_t *buf, uint32_t buf_len) { open(buf, buf_len); }

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

            uint32_t length();

            [[nodiscard]] bool isOpen() const {
                return buf != nullptr || fh != nullptr;
            }

            [[nodiscard]] const uint8_t *getPtr() const {
                return buf;
            }

            static void addBufferFile(const std::string &path, const uint8_t *ptr, uint32_t len);

        private:
            void *fh = nullptr;
            // buffer "files"
            const uint8_t *buf = nullptr;
            uint32_t buf_len{};
            bool is_cached = false;
        };

        Io();

        virtual ~Io() = default;

#warning "TODO: is_device_available"
        //bool is_device_available(const Device &device);

        std::vector<FileInfo> list(const std::string &path, std::function<bool(const FileInfo &)> filter = nullptr);

        bool copy(const File &src, const File &dst);

        bool copy(const std::string &src, const std::string &dst);

        bool remove(const std::string &path);

        bool fileExists(const std::string &path);

        virtual bool directoryExists(const std::string &path) { return false; }

        virtual bool create(const std::string &path) { return false; }

        virtual bool rename(const std::string &old_name, const std::string &new_name) { return false; }

        virtual bool format(const Device &device) { return false; }

        virtual bool is_files_open_priv() { return false; }

    protected:
        virtual void *open_file_priv(const std::string &file, int mode) { return nullptr; };

        virtual int32_t read_file_priv(void *fh, uint32_t offset, uint32_t length, char *buffer) { return -1; }

        virtual int32_t write_file_priv(void *fh, uint32_t offset, uint32_t length, const char *buffer) { return -1; }

        virtual int32_t close_file_priv(void *fh) { return -1; }

        virtual uint32_t get_file_length_priv(void *fh) { return -1; }

        virtual void list_files_priv(const std::string &path, std::function<void(FileInfo &)> callback) {}

        virtual bool file_exists_priv(const std::string &path) { return false; }

        virtual bool remove_file_priv(const std::string &path) { return false; }

        virtual void close_open_files_priv() {}
    };
}

#endif //PICO2D_IO_H
