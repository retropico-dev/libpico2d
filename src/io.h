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

        struct FileBuffer {
            const uint8_t *ptr{};
            uint32_t length{};
        };

        struct ListBuffer {
#ifdef LINUX
            std::vector<std::string> data;
#else
            uint8_t *data = nullptr;
#endif
            uint32_t data_size = 0;

            uint16_t count = 0;

            [[nodiscard]] char *at(int idx) const {
#ifdef LINUX
                return const_cast<char *>(data.at(idx).c_str());
#else
                return (char *) &data[idx * IO_MAX_PATH];
#endif
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

            static void addBufferFile(const std::string &path, const uint8_t *ptr, uint32_t len);

        private:
            std::string m_path;
            std::string m_name;
            void *p_fh = nullptr;
            const uint8_t *buf = nullptr;
            uint32_t buf_len{};
        };

        Io() = default;

        ~Io() = default;

        virtual bool isAvailable(const Device &device) { return false; }

        virtual bool create(const std::string &path) { return false; }

        virtual bool remove(const std::string &path) { return false; }

        virtual bool rename(const std::string &old_name, const std::string &new_name) { return false; }

        virtual bool fileExists(const std::string &path) { return false; }

        virtual bool directoryExists(const std::string &path) { return false; }

        virtual bool copy(const File &src, const File &dst) { return false; }

        virtual bool copy(const std::string &src, const std::string &dst) { return false; }

        virtual std::vector<File::Info> getList(
                const std::string &path, std::function<bool(const File::Info &)> const &filter = nullptr) {
            return {};
        }

        virtual ListBuffer getBufferedList(const std::string &path, uint32_t flashOffset) {
            return {};
        }

        virtual bool format(const Device &device) { return false; }
    };
}

#endif //PICO2D_IO_H
