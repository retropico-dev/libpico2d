//
// Created by cpasjuste on 20/01/24.
//

#include <cstring>
#include <map>
#include "io.h"

using namespace p2d;

extern std::map<std::string, Io::BufferFile> p2d_io_buf_files;

bool File::open(const uint8_t *b, uint32_t b_len) {
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
bool File::open(const std::string &file, int mode) {
    close();

    // check for buffer
    auto it = p2d_io_buf_files.find(file);

    if (!(mode & OpenMode::Write) && it != p2d_io_buf_files.end()) {
        buf = it->second.ptr;
        buf_len = it->second.length;
        return true;
    }

    fh = Io::FatFs::open_file(file, mode);
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
int32_t File::read(uint32_t offset, uint32_t length, char *buffer) const {
    if (buf) {
        auto len = std::min(length, buf_len - offset);
        memcpy(buffer, buf + offset, len);
        return (int32_t) len;
    }

    return Io::FatFs::read_file(fh, offset, length, buffer);
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
int32_t File::write(uint32_t offset, uint32_t length, const char *buffer) const {
    return Io::FatFs::write_file(fh, offset, length, buffer);
}

/**
 * Close the file. Also called automatically by the destructor.
 */
void File::close() {
    buf = nullptr;

    if (!fh)
        return;

    Io::FatFs::close_file(fh);
    fh = nullptr;
}

/**
 * Get file length
 *
 * \return Length of the file in bytes.
 */
uint32_t File::length() {
    if (buf) return buf_len;

    return Io::FatFs::get_file_length(fh);
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
void File::addBufferFile(const std::string &path, const uint8_t *ptr, uint32_t len) {
    p2d_io_buf_files.emplace(path, Io::BufferFile{ptr, len});
}
