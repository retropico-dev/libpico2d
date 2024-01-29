// Created by cpasjuste on 01/06/23.
//

#include <cstring>
#include <algorithm>
#include <map>
#include "cmrc/cmrc.hpp"
#include "platform.h"
#include "ff.h"
#include "diskio.h"

#ifndef FLASH_SECTOR_SIZE
#define FLASH_SECTOR_SIZE 4096
#endif

#ifndef XIP_BASE
#define XIP_BASE 0
#endif

using namespace p2d;
using namespace cmrc;
CMRC_DECLARE(pico2d);

static FATFS sd_fs;
static FATFS flash_fs;
static bool io_initialised = false;
static bool sd_initialised = false;
static std::vector<void *> open_files;
std::map<std::string, Io::FileBuffer> buf_files; // TODO: refactor/remove this

// hacky/crappy core1 "fix"
extern void p2d_display_pause();

extern void p2d_display_resume();

static void getResources(const embedded_filesystem &fs, const std::string &path) {
    for (auto &&entry: fs.iterate_directory(path)) {
        std::string p = path + "/" + entry.filename();
        if (entry.is_directory()) {
            getResources(fs, p);
        } else {
            auto file = fs.open(p);
            Io::File::addBufferFile("res:" + p, (const uint8_t *) file.cbegin(), file.size());
            printf("Io: resource added: \"%s\"\r\n", std::string("res:" + p).c_str());
        }
    }
}

void Io::init() {
    if (io_initialised) return;
    io_initialised = true;

    // init sdcard
    printf("Io: mounting sdcard fs...\r\n");
    auto res = f_mount(&sd_fs, "sd:", 1);
    if (res != FR_OK) {
        printf("Io: failed to mount sdcard filesystem! (%i)\r\n", res);
#if defined(LINUX) || defined(FORCE_FORMAT_SD)
        if (res == FR_NO_FILESYSTEM) {
            printf("Io: no filesystem found on sdcard, formatting...\r\n");
            format(Device::Sd);
        }
#endif
    } else {
        printf("Io: mounted sdcard fs on \"sd:\" (%s)\r\n",
               io_sdcard_get_size_string().c_str());
    }

    // mount flash fs
    printf("Io: mounting flash fs...\r\n");
    res = f_mount(&flash_fs, "flash:", 1);
    if (res != FR_OK) {
        printf("Io: failed to mount flash filesystem! (%i)\r\n", res);
        if (res == FR_NO_FILESYSTEM) {
            printf("Io: no filesystem found on flash, formatting...\r\n");
            format(Device::Flash);
        }
    } else {
        printf("Io: mounted flash fs on \"flash:\" (%s)\r\n",
               io_flash_get_size_string().c_str());
    }

#warning TODO
    // TODO: fix "undefined reference to `cmrc::pico2d::get_filesystem()'"
    //  when no resources added from cmake project (edit: release mode/flags bug?)
    // map "romfs" (resources)
    auto fs = pico2d::get_filesystem();
    getResources(fs, "");

}

void Io::exit() {
    printf("Io::~Io()\r\n");

    if (sd_initialised) {
        if (FatFs::is_files_open()) {
            FatFs::close_open_files();
        }

        f_unmount("sd:");
        io_sdcard_exit();

        f_unmount("flash:");
        io_flash_exit();
    }
}

bool Io::isAvailable(const Io::Device &device) {
    if (device == Device::Flash) return true;
    return sd_initialised;
}

bool Io::create(const std::string &path) {
    FRESULT fr;
    const char *p;
    char *temp;

    // add "/"
    std::string newPath = path;
    if (newPath[newPath.size() - 1] != '/') {
        newPath = newPath + "/";
    }

    temp = static_cast<char *>(calloc(1, strlen(newPath.c_str()) + 1));
    p = newPath.c_str();

    while ((p = strchr(p, '/')) != nullptr) {
        if (p != newPath.c_str() && *(p - 1) == '/') {
            p++;
            continue;
        }
        memcpy(temp, newPath.c_str(), p - newPath.c_str());
        temp[p - newPath.c_str()] = '\0';
        p++;
        fr = f_mkdir(temp);
        if (fr != FR_OK && fr != FR_EXIST) {
            break;
        }
    }

    free(temp);

    return fr == FR_OK || fr == FR_EXIST;
}

bool Io::rename(const std::string &old_name, const std::string &new_name) {
    return f_rename(old_name.c_str(), new_name.c_str()) == FR_OK;
}

bool Io::remove(const std::string &path) {
    auto it = buf_files.find(path);
    if (it != buf_files.end()) {
        buf_files.erase(it);
        return true;
    }

    return FatFs::remove_file(path);
}

bool Io::directoryExists(const std::string &path) {
    FILINFO info;
    return f_stat(path.c_str(), &info) == FR_OK && (info.fattrib & AM_DIR);
}

bool Io::fileExists(const std::string &path) {
    return FatFs::file_exists(path) || buf_files.find(path) != buf_files.end();
}

bool Io::format(const Io::Device &device) {
    FRESULT res;
    MKFS_PARM opts{.fmt =  FM_ANY | FM_SFD};
    std::string path = device == Device::Flash ? "flash:" : "sd:";
    uint32_t sector_size = device == Device::Flash ? FLASH_SECTOR_SIZE : FF_MIN_SS;
    auto fs = device == Device::Flash ? &flash_fs : &sd_fs;

    printf("Io::format: formatting drive \"%s\"...\r\n", path.c_str());

    res = f_mkfs(path.c_str(), &opts, fs->win, sector_size);
    if (res != FR_OK) {
        printf("Io::format: format failed! (%i)\r\n", res);
        return false;
    }

    res = f_mount(fs, path.c_str(), 1);
    if (res != FR_OK) {
        printf("Io::format: failed to mount filesystem! (%i)\r\n", res);
        return false;
    }

    if (device == Device::Flash) {
        printf("Io: mounted flash fs on \"flash:\" (%s)\r\n",
               io_flash_get_size_string().c_str());
    } else {
        printf("Io: mounted sdcard fs on \"sd:\" (%s)\r\n",
               io_flash_get_size_string().c_str());
    }

    return true;
}

bool Io::copy(const File &src, const File &dst) {
    int32_t read;
    int32_t offset = 0;
    char *read_buffer;

    if (!src.isOpen()) {
        printf("Io::copy: could not open source file\r\n");
        return false;
    }

    if (!dst.isOpen()) {
        printf("Io::copy: could not open destination file\r\n");
        return false;
    }

    // TODO: fix this "randomly" not working
    // use a contiguous region for flash for later raw access
    if (Utility::startWith(dst.getPath(), "flash:")) {
        auto r = f_expand((FIL *) dst.getHandler(), src.getLength(), 0);
        if (r != FRESULT::FR_OK) {
            printf("Io::copy: WARNING: could not put file in contiguous data area in flash\r\n");
        }
    }

    read_buffer = (char *) malloc(4096);

    while ((read = src.read(offset, 4096, read_buffer)) > 0) {
        dst.write(offset, read, read_buffer);
        offset += read;
    }

    free(read_buffer);

    printf("Io::copy: copied %i bytes\r\n", offset);
    return true;
}

bool Io::copy(const std::string &src, const std::string &dst) {
    bool core1_pause_needed = Utility::startWith(dst, "flash:");
    if (core1_pause_needed) {
        p2d_display_pause();
        sleep_ms(20);
    }

    const File srcFile = File{src, File::OpenMode::Read};
    const File dstFile = File{dst, File::OpenMode::Write};
    auto res = copy(srcFile, dstFile);

    if (core1_pause_needed) {
        p2d_display_resume();
        sleep_ms(20);
    }

    return res;
}

std::vector<Io::File::Info> Io::getList(
        const std::string &path, std::function<bool(const File::Info &)> const &filter) {
    std::vector<File::Info> ret;
    FatFs::list_files(path, [&ret, &filter](File::Info &file) {
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
            File::Info info = {};
            info.name = buf_file.first.substr(slash_pos == std::string::npos ? 0 : slash_pos + 1);
            info.size = buf_file.second.length;
            ret.push_back(info);
        }
    }

    return ret;
}

// TODO: handle directories ?
Io::ListBuffer in_ram(Io::getBufferedList)(const std::string &path, uint32_t flashOffset) {
    char buffer[FLASH_SECTOR_SIZE / IO_MAX_PATH][IO_MAX_PATH];
    uint32_t maxFilesPerWrite = FLASH_SECTOR_SIZE / IO_MAX_PATH;
    uint32_t offset = flashOffset;
    uint32_t count = 0, currentFile = 0;
    Io::ListBuffer listBuffer;

    p2d_display_pause();

    FatFs::list_files(path, [&count, &currentFile, &offset, &maxFilesPerWrite, &buffer](File::Info &file) {
        if (!(file.flags & File::Flags::Directory)) {
            strncpy(buffer[currentFile], file.name.c_str(), IO_MAX_PATH - 1);
            count++;
            currentFile++;
            if (currentFile == maxFilesPerWrite) {
                io_flash_write_sector(offset, (const uint8_t *) buffer);
                offset += FLASH_SECTOR_SIZE;
                currentFile = 0;
            }
        }
    });

    // flash remaining
    if (currentFile > 0) {
        io_flash_write_sector(offset, (const uint8_t *) buffer);
        offset += FLASH_SECTOR_SIZE;
    }

    listBuffer.data_size = offset - flashOffset;
    listBuffer.data = (uint8_t *) (XIP_BASE + flashOffset);
    listBuffer.count = (int) count;

    p2d_display_resume();

    return listBuffer;
}

///
/// Io::File functions
///
static LBA_t clst2sect(FATFS *fs, DWORD clst) {
    clst -= 2;
    if (clst >= fs->n_fatent - 2) return 0;
    return fs->database + (LBA_t) fs->csize * clst;
}

bool Io::File::open(const uint8_t *b, uint32_t b_len) {
    close();

    this->buf = b;
    this->buf_len = b_len;
    return true;
}

bool Io::File::open(const std::string &path, int mode) {
    close();

    // check for buffer
    auto it = buf_files.find(path);

    if (!(mode & OpenMode::Write) && it != buf_files.end()) {
        buf = it->second.ptr;
        buf_len = it->second.length;
        return true;
    }

    p_fh = Io::FatFs::open_file(path, mode);

    // allow accessing raw flash file at correct offset when opened in read mode
    if (!(mode & OpenMode::Write) && Utility::startWith(path, "flash:")) {
        FIL *fp = (FIL *) p_fh;
        uint32_t sector = clst2sect(fp->obj.fs, fp->obj.sclust);
        buf = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET_FATFS + (sector * FLASH_SECTOR_SIZE));
        buf_len = fp->obj.objsize;
    }

    return p_fh != nullptr;
}

int32_t Io::File::read(uint32_t offset, uint32_t length, char *buffer) const {
    if (buf) {
        auto len = std::min(length, buf_len - offset);
        memcpy(buffer, buf + offset, len);
        return (int32_t) len;
    }

    return Io::FatFs::read_file(p_fh, offset, length, buffer);
}

int32_t Io::File::write(uint32_t offset, uint32_t length, const char *buffer) const {
    return Io::FatFs::write_file(p_fh, offset, length, buffer);
}

void Io::File::close() {
    buf = nullptr;

    if (!p_fh) return;

    Io::FatFs::close_file(p_fh);
    p_fh = nullptr;
}

uint32_t Io::File::getLength() const {
    if (buf) return buf_len;
    return Io::FatFs::get_file_length(p_fh);
}

void Io::File::addBufferFile(const std::string &path, const uint8_t *ptr, uint32_t len) {
    const Io::FileBuffer fb = Io::FileBuffer{ptr, len};
    buf_files.emplace(path, fb);
}

///
/// FatFs high level io functions
///

void *Io::FatFs::open_file(const std::string &path, int mode) {
    FIL *f = new FIL();
    BYTE ff_mode = 0;

    if (mode & File::OpenMode::Read)
        ff_mode |= FA_READ;
    if (mode & File::OpenMode::Write)
        ff_mode |= FA_WRITE;
    if (mode == File::OpenMode::Write)
        ff_mode |= FA_CREATE_ALWAYS;

    FRESULT r = f_open(f, path.c_str(), ff_mode);
    if (r == FR_OK) {
        open_files.push_back(f);
        return f;
    }

    delete f;
    return nullptr;
}

int32_t Io::FatFs::read_file(void *fh, uint32_t offset, uint32_t length, char *buffer) {
    FRESULT r = FR_OK;
    FIL *f = (FIL *) fh;

    if (offset != f_tell(f))
        r = f_lseek(f, offset);

    if (r == FR_OK) {
        unsigned int bytes_read;
        r = f_read(f, buffer, length, &bytes_read);
        if (r == FR_OK) {
            return (int32_t) bytes_read;
        }
    }

    return -1;
}

int32_t Io::FatFs::write_file(void *fh, uint32_t offset, uint32_t length, const char *buffer) {
    FRESULT r = FR_OK;
    FIL *f = (FIL *) fh;

    if (offset != f_tell(f))
        r = f_lseek(f, offset);

    if (r == FR_OK) {
        unsigned int bytes_written;
        r = f_write(f, buffer, length, &bytes_written);
        if (r == FR_OK) {
            return (int32_t) bytes_written;
        }
    }

    return -1;
}

int32_t Io::FatFs::close_file(void *fh) {
    FRESULT r;

    r = f_close((FIL *) fh);

    for (auto it = open_files.begin(); it != open_files.end(); ++it) {
        if (*it == fh) {
            open_files.erase(it);
            break;
        }
    }

    delete (FIL *) fh;
    return r == FR_OK ? 0 : -1;
}

uint32_t Io::FatFs::get_file_length(void *fh) {
    return f_size((FIL *) fh);
}

void Io::FatFs::list_files(const std::string &path, std::function<void(File::Info &)> callback) {
    DIR dir;

    if (f_opendir(&dir, path.c_str()) != FR_OK)
        return;

    FILINFO ent;

    while (f_readdir(&dir, &ent) == FR_OK && ent.fname[0]) {
        File::Info info{
                .name = ent.fname,
                .flags = 0,
                .size = ent.fsize
        };

        if (ent.fattrib & AM_DIR)
            info.flags |= File::Flags::Directory;

        callback(info);
    }

    f_closedir(&dir);
}

bool Io::FatFs::file_exists(const std::string &path) {
    FILINFO info;
    return f_stat(path.c_str(), &info) == FR_OK && !(info.fattrib & AM_DIR);
}

bool Io::FatFs::remove_file(const std::string &path) {
    return f_unlink(path.c_str()) == FR_OK;
}

bool Io::FatFs::is_files_open() {
    return open_files.size() > 0;
}

void Io::FatFs::close_open_files() {
    while (!open_files.empty()) close_file(open_files.back());
}

///
/// FatFs low level io functions
///

DSTATUS disk_initialize(BYTE drv) {
    //printf("disk_initialize: %i\r\n", drv);
    if (drv == Io::Device::Sd) {
        if (sd_initialised) return RES_OK;
        sd_initialised = io_sdcard_init();
        return sd_initialised ? RES_OK : STA_NOINIT;
    }

    // flash always available...
    io_flash_init();
    return RES_OK;
}

DSTATUS disk_status(BYTE drv) {
    //printf("disk_status: %i\r\n", drv);
    if (drv == Io::Device::Sd) {
        return sd_initialised ? RES_OK : STA_NOINIT;
    }

    // flash always available...
    return RES_OK;
}

DRESULT disk_read(BYTE drv, BYTE *buff, LBA_t sector, UINT count) {
    //printf("disk_read: %i\r\n", drv);
    if (drv == Io::Device::Sd) {
        return io_sdcard_read(sector, 0, buff, FF_MIN_SS * count)
               == int32_t(FF_MIN_SS * count) ? RES_OK : RES_ERROR;
    }

    return io_flash_read(sector, 0, buff, FLASH_SECTOR_SIZE * count)
           == int32_t(FLASH_SECTOR_SIZE * count) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE drv, const BYTE *buff, LBA_t sector, UINT count) {
#if 0
    printf("disk_write: %i, address: 0x%08lx\r\n", drv,
           XIP_BASE + FLASH_TARGET_OFFSET_FATFS + ( sector * FLASH_SECTOR_SIZE));
#endif
    if (drv == Io::Device::Sd) {
        return io_sdcard_write(sector, 0, buff, FF_MIN_SS * count)
               == int32_t(FF_MIN_SS * count) ? RES_OK : RES_ERROR;
    }

    return io_flash_write(sector, 0, buff, FLASH_SECTOR_SIZE * count)
           == int32_t(FLASH_SECTOR_SIZE * count) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE drv, BYTE cmd, void *buff) {
    //printf("disk_ioctl: drv: %i, cmd: %i\r\n", drv, cmd);
    uint16_t block_size;
    uint32_t num_blocks;

    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            if (drv == Io::Device::Sd) {
                io_sdcard_get_size(block_size, num_blocks);
            } else {
                io_flash_get_size(block_size, num_blocks);
            }
            *(LBA_t *) buff = num_blocks;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *(WORD *) buff = drv == Io::Device::Sd ? FF_MIN_SS : FLASH_SECTOR_SIZE;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *(DWORD *) buff = 1;
            return RES_OK;
        default:
            break;
    }

    return RES_PARERR;
}
