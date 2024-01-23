//
// Created by cpasjuste on 23/01/24.
//

#ifndef PICO2D_SKELETON_IO_PICO_H
#define PICO2D_SKELETON_IO_PICO_H

namespace p2d {
    class PicoIo : public Io {
    public:
        PicoIo();

        ~PicoIo();

        bool isAvailable(const Device &device) override;

        std::vector<File::Info> getList(
                const std::string &path, std::function<bool(const File::Info &)> const &filter = nullptr) override;

        ListBuffer getBufferedList(const std::string &path, uint32_t flashOffset) override;

        bool copy(const File &src, const File &dst) override;

        bool copy(const std::string &src, const std::string &dst) override;

        bool remove(const std::string &path) override;

        bool fileExists(const std::string &path) override;

        bool directoryExists(const std::string &path) override;

        bool create(const std::string &path) override;

        bool rename(const std::string &old_name, const std::string &new_name) override;

        bool format(const Device &device) override;

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

#endif //PICO2D_SKELETON_IO_PICO_H
