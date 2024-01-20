//
// Created by cpasjuste on 01/06/23.
//

#ifndef MICROBOY_IO_LINUX_H
#define MICROBOY_IO_LINUX_H

namespace p2d {
    class LinuxIo : public Io {
    public:
        File read(const std::string &path, const Target &target = Flash) override;

        bool write(const std::string &path, const File &fileBuffer) override;

        bool writeRomToFlash(const std::string &path, const std::string &name) override;

        File readRomFromFlash() override;

        FileListBuffer getDir(const std::string &path) override;

        void createDir(const std::string &path) override;
    };
}

#endif //MICROBOY_IO_LINUX_H
