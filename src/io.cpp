//
// Created by cpasjuste on 19/01/24.
//

// https://github.com/32blit/32blit-sdk/blob/master/32blit/engine/file.cpp

#include <algorithm>
#include <map>
#include "io.h"

using namespace p2d;

std::map<std::string, Io::BufferFile> p2d_io_buf_files;

/**
 * Check if it is possible to read/write files, for SDL this is always true.
 *
 * \return true if an SD card is inserted and usable, false otherwise
 */
/*
bool is_storage_available() {
   return api.is_storage_available();
}
*/

/**
 * Lists files on the SD card (device), the game directory (SDL) or in memory.
 *
 * \param path Path to list files at, relative to the root of the SD card or game directory (SDL).
 *
 * \return Vector of files/directories
 */
std::vector<File::Info> Io::list(const std::string &path, std::function<bool(const File::Info &)> filter) {
    std::vector<File::Info> ret;
    FatFs::list_files(path, [&ret, &filter](File::Info &file) {
        if (!filter || filter(file))
            ret.push_back(file);
    });

    for (auto &buf_file: p2d_io_buf_files) {
        auto slash_pos = buf_file.first.find_last_of('/');

        bool match;
        if (slash_pos == std::string::npos) // file in root
            match = path.empty() || path == "/";
        else {
            if (!path.empty() && path.back() == '/') // path has trailing slash
                match = buf_file.first.substr(0, slash_pos + 1) == path;
            else
                match = buf_file.first.substr(0, slash_pos) == path;
        }

        if (match) {
            File::Info info = {};
            info.name = buf_file.first.substr(slash_pos == std::string::npos ? 0 : slash_pos + 1);
            info.size = buf_file.second.length;
            ret.push_back(info);
        }
    }

    return ret;
}

/**
 * Check if the specified path fileExists and is a file
 *
 * \param path Path to check existence of, relative to the root of the SD card (device) or game directory (SDL).
 *
 * \return true if file fileExists
 */
bool Io::fileExists(const std::string &path) {
    return FatFs::file_exists(path) || p2d_io_buf_files.find(path) != p2d_io_buf_files.end();
}

/**
 * Remove a file
 *
 * \param path Path to remove, relative to the root of the SD card (device) or game directory (SDL).
 *
 * \return true if file removed successfully
 */
bool Io::remove(const std::string &path) {
    auto it = p2d_io_buf_files.find(path);
    if (it != p2d_io_buf_files.end()) {
        p2d_io_buf_files.erase(it);
        return true;
    }

    return FatFs::remove_file(path);
}

bool Io::copy(const File &src, const File &dst) {
    char buf[4096];
    int32_t read;
    int32_t offset = 0;

    if (!src.isOpen()) {
        printf("Io::copy: could not open source file\r\n");
        return false;
    }

    if (!dst.isOpen()) {
        printf("Io::copy: could not open destination file\r\n");
        return false;
    }

    while ((read = src.read(offset, 4096, buf)) > 0) {
        dst.write(offset, read, buf);
        offset += read;
    }

    printf("Io::copy: copied %li bytes\r\n", offset);
    return true;
}

bool Io::copy(const std::string &src, const std::string &dst) {
    const File srcFile = File{src, File::OpenMode::Read};
    const File dstFile = File{dst, File::OpenMode::Write};
    return copy(srcFile, dstFile);
}
