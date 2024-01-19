//
// Created by cpasjuste on 01/06/23.
//

#ifndef MICROBOY_IO_H
#define MICROBOY_IO_H

#include <cstdint>
#include <string>
#include <vector>

#define IO_MAX_FILES 2048
#define IO_MAX_PATH 128

namespace p2d {
    class Io {
    public:
        enum Device {
            Flash,
            Sd,
            //Ram
        };

        struct FileBuffer {
            char name[IO_MAX_PATH]{};
            uint8_t *data = nullptr;
            size_t size = 0;
            bool rw = false;
        };

        struct FileBufferList {
#ifdef LINUX
            std::vector<std::string> data;
#else
            uint8_t *data = nullptr;
#endif
            int count = 0;

            [[nodiscard]] char *get(int idx) const {
#ifdef LINUX
                return const_cast<char *>(data.at(idx).c_str());
#else
                return (char *) &data[idx * IO_MAX_PATH];
#endif
            }
        };

        Io() = default;

        virtual ~Io() {
            printf("~Io()\n");
        }

        virtual FileBuffer read(const std::string &path, const Device &target = Flash) { return {}; }

        virtual FileBuffer readRomFromFlash() { return {}; }

        virtual bool write(const std::string &path, const FileBuffer &fileBuffer) { return false; }

        virtual bool writeRomToFlash(const std::string &path, const std::string &name) { return false; }

        virtual FileBufferList getDir(const std::string &path) { return {}; }

        virtual void createDir(const std::string &path) {};
    };
}

#endif //MICROBOY_IO_H
