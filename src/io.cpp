//
// Created by cpasjuste on 19/01/24.
//

// https://github.com/32blit/32blit-sdk/blob/master/32blit/engine/file.cpp

#include <algorithm>
#include <cstring>
#include <map>

#include "io.h"

using namespace p2d;

struct BufferFile {
    const uint8_t *ptr;
    uint32_t length;
};

static std::map<std::string, BufferFile> buf_files;

static Io *s_io;

Io::Io() {
    s_io = this;
}

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
std::vector<Io::FileInfo> Io::list(const std::string &path, std::function<bool(const FileInfo &)> filter) {
    std::vector<FileInfo> ret;
    list_files_priv(path, [&ret, &filter](FileInfo &file) {
        if (!filter || filter(file))
            ret.push_back(file);
    });

    for (auto &buf_file: buf_files) {
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
            FileInfo info = {};
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
    return file_exists_priv(path) || buf_files.find(path) != buf_files.end();
}

/**
 * Remove a file
 *
 * \param path Path to remove, relative to the root of the SD card (device) or game directory (SDL).
 *
 * \return true if file removed successfully
 */
bool Io::remove(const std::string &path) {
    auto it = buf_files.find(path);
    if (it != buf_files.end()) {
        buf_files.erase(it);
        return true;
    }
    return remove_file_priv(path);
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
    const File srcFile = File{src, OpenMode::Read};
    const File dstFile = File{dst, OpenMode::Write};
    return copy(srcFile, dstFile);
}

bool Io::File::open(const uint8_t *b, uint32_t b_len) {
    close();

    this->buf = b;
    this->buf_len = b_len;
    return true;
}

/**
 * Open a file. If a file is already open it will be automatically closed.
 *
 * \param file Path to open.
 * \param mode ::OpenMode to open file as. Cannot contain ::write for in-memory files.
 *
 * \return true if file opened successfully
 */
bool Io::File::open(const std::string &file, int mode) {
    close();

    // check for buffer
    auto it = buf_files.find(file);

    if (!(mode & OpenMode::Write) && it != buf_files.end()) {
        buf = it->second.ptr;
        buf_len = it->second.length;
        return true;
    }

    fh = s_io->open_file_priv(file, mode);
    return fh != nullptr;
}

/**
 * Read a block of data from the file. Should not be called if the file was not opened for reading.
 *
 * \param offset Offset to read from
 * \param length Length to read
 * \param buffer Pointer to buffer to store data into, should be at least `length` bytes
 *
 * \return Number of bytes read successfully or -1 if an error occurred.
 */
int32_t Io::File::read(uint32_t offset, uint32_t length, char *buffer) const {
    if (buf) {
        auto len = std::min(length, buf_len - offset);
        memcpy(buffer, buf + offset, len);
        return (int32_t) len;
    }

    return s_io->read_file_priv(fh, offset, length, buffer);
}

/**
 * Write a block of data to the file. Should not be called if the file was not opened for writing.
 *
 * \param offset Offset to write to
 * \param length Length to write
 * \param buffer Pointer to data to write, should be at least `length` bytes
 *
 * \return Number of bytes written successfully or -1 if an error occurred.
 */
int32_t Io::File::write(uint32_t offset, uint32_t length, const char *buffer) const {
    return s_io->write_file_priv(fh, offset, length, buffer);
}

/**
 * Close the file. Also called automatically by the destructor.
 */
void Io::File::close() {
    buf = nullptr;

    if (!fh)
        return;

    s_io->close_file_priv(fh);
    fh = nullptr;
}

/**
 * Get file length
 *
 * \return Length of the file in bytes.
 */
uint32_t Io::File::length() {
    if (buf) return buf_len;

    return s_io->get_file_length_priv(fh);
}

/**
 * Creates an in-memory file, which can be used like a regular (read-only) file.
 *
 * This is useful for porting code which assumes files, or for transparently moving data to flash
 * for extra performance.
 *
 * Example using a packed asset:
 * ```
 * File::addBufferFile("asset_name.bin", asset_name, asset_name_length);
 * ```
 *
 * Notes:
 * The directory part of the path is not created if it does not exist, so ::list/::directoryExists
 * may not work as expected in that case: (Assuming `path/to` does not exist on the SD card)
 * ```
 * File::addBufferFile("path/to/a.file");
 *
 * fileExists("path/to/a.file"); // true
 * directoryExists("path/to"); // false!
 *
 * list("path/to"); // vector containing info for "a.file"
 * list("path"); // empty!
 * ```
 *
 * \param path Path for the file
 * \param ptr Pointer to file data
 * \param len Length of file data
 */
void Io::File::addBufferFile(const std::string &path, const uint8_t *ptr, uint32_t len) {
    buf_files.emplace(path, BufferFile{ptr, len});
}
