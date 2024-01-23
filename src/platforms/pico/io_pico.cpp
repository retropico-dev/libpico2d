// Created by cpasjuste on 01/06/23.
//

#include <cstring>
#include "platform.h"
#include "flash.h"
#include "sdcard.h"
#include "ff.h"
#include "diskio.h"

using namespace p2d;

static FATFS sd_fs;
static FATFS flash_fs;
static bool io_initialised = false;
static bool sd_initialised = false;
static std::vector<void *> open_files;

void Io::init() {
    if (io_initialised) return;
    io_initialised = true;

    // init sdcard
    printf("PicoIo: mounting sdcard fs...\r\n");
    auto res = f_mount(&sd_fs, "sd:", 1);
    if (res != FR_OK) {
        printf("PicoIo: failed to mount sdcard filesystem! (%i)\r\n", res);
#ifdef FORCE_FORMAT_SD
        if (res == FR_NO_FILESYSTEM) {
            printf("PicoIo: no filesystem found on sdcard, formatting...\r\n");
            format(Device::Sd);
        }
#endif
    } else {
        printf("PicoIo: mounted sdcard fs on \"sd:\" (%s)\r\n",
               io_sdcard_get_size_string().c_str());
    }

    // mount flash fs
    printf("PicoIo: mounting flash fs...\r\n");
    res = f_mount(&flash_fs, "flash:", 1);
    if (res != FR_OK) {
        printf("PicoIo: failed to mount flash filesystem! (%i)\r\n", res);
        if (res == FR_NO_FILESYSTEM) {
            printf("PicoIo: no filesystem found on flash, formatting...\r\n");
            format(Device::Flash);
        }
    } else {
        printf("PicoIo: mounted flash fs on \"flash:\" (%s)\r\n",
               io_flash_get_size_string().c_str());
    }
}

bool Io::directoryExists(const std::string &path) {
    FILINFO info;
    return f_stat(path.c_str(), &info) == FR_OK && (info.fattrib & AM_DIR);
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

// TODO: handle directories
Io::ListBuffer in_ram(Io::getBufferedList)(const std::string &path, uint32_t flashOffset) {
    char buffer[FLASH_SECTOR_SIZE / IO_MAX_PATH][IO_MAX_PATH];
    uint32_t maxFilesPerWrite = FLASH_SECTOR_SIZE / IO_MAX_PATH;
    uint32_t offset = flashOffset;
    uint32_t count = 0, currentFile = 0;
    Io::ListBuffer listBuffer;

    std::vector<File::Info> ret;
    FatFs::list_files(path, [&count, &currentFile, &offset, &maxFilesPerWrite, &buffer](File::Info &file) {
        if (!(file.flags & File::Flags::Directory)) {
            strncpy(buffer[currentFile], file.name.c_str(), IO_MAX_PATH - 1);
            currentFile++;
            count++;
            if (currentFile == maxFilesPerWrite) {
                io_flash_write_sector(offset, (const uint8_t *) buffer);
                offset += FLASH_SECTOR_SIZE;
                currentFile = 0;
                memset(buffer, 0, sizeof(buffer));
            }
        }
    });

    // flash remaining
    if (currentFile > 0) {
        io_flash_write_sector(offset, (const uint8_t *) buffer);
    }

    listBuffer.data = (uint8_t *) (XIP_BASE + flashOffset);
    listBuffer.data_size = count * IO_MAX_PATH;
    listBuffer.count = (int) count;

    return listBuffer;
}

bool Io::rename(const std::string &old_name, const std::string &new_name) {
    return f_rename(old_name.c_str(), new_name.c_str()) == FR_OK;
}

bool Io::format(const Io::Device &device) {
    FRESULT res;
    MKFS_PARM opts{.fmt =  FM_ANY | FM_SFD};
    std::string path = device == Device::Flash ? "flash:" : "sd:";
    uint32_t sector_size = device == Device::Flash ? FLASH_SECTOR_SIZE : FF_MIN_SS;
    auto fs = device == Device::Flash ? &flash_fs : &sd_fs;

    printf("PicoIo::format: formatting drive \"%s\"...\r\n", path.c_str());

    res = f_mkfs(path.c_str(), &opts, fs->win, sector_size);
    if (res != FR_OK) {
        printf("PicoIo::format: format failed! (%i)\r\n", res);
        return false;
    }

    res = f_mount(fs, path.c_str(), 1);
    if (res != FR_OK) {
        printf("PicoIo::format: failed to mount filesystem! (%i)\r\n", res);
    }

    printf("PicoIo: mounted flash fs on \"flash:\" (%s)\r\n",
           io_flash_get_size_string().c_str());

    return true;
}

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

bool Io::isAvailable(const Io::Device &device) {
    if (device == Device::Flash) return true;
    return sd_initialised;
}

bool Io::FatFs::is_files_open() {
    return open_files.size() > 0;
}

void Io::FatFs::close_open_files() {
    while (!open_files.empty()) close_file(open_files.back());
}

void Io::destroy() {
#warning "TODO: ~PicoIo: unmount stuff"
}

// fatfs io funcs
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
