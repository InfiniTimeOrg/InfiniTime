#pragma once

#include <cstdint>
#include "drivers/SpiNorFlash.h"
#include <littlefs/lfs.h>

namespace Pinetime {
  namespace Controllers {
    class FS {
    public:
      FS(Pinetime::Drivers::SpiNorFlash&);

      void Init();

      int FileOpen(lfs_file_t* file_p, const char* fileName, const int flags);
      int FileClose(lfs_file_t* file_p);
      int FileRead(lfs_file_t* file_p, uint8_t* buff, uint32_t size);
      int FileWrite(lfs_file_t* file_p, const uint8_t* buff, uint32_t size);
      int FileSeek(lfs_file_t* file_p, uint32_t pos);

      int FileDelete(const char* fileName);

      int DirOpen(const char* path, lfs_dir_t* lfs_dir);
      int DirClose(lfs_dir_t* lfs_dir);
      int DirRead(lfs_dir_t* dir, lfs_info* info);
      int DirRewind(lfs_dir_t* dir);
      int DirCreate(const char* path);

      lfs_ssize_t GetFSSize();
      int Rename(const char* oldPath, const char* newPath);
      int Stat(const char* path, lfs_info* info);
      void VerifyResource();

      static size_t getSize() {
        return size;
      }

      static size_t getBlockSize() {
        return blockSize;
      }

    private:
      Pinetime::Drivers::SpiNorFlash& flashDriver;

      /*
       * External Flash MAP (4 MBytes)
       *
       * 0x000000 +---------------------------------------+
       *          |  Bootloader Assets                    |
       *          |  256 KBytes                           |
       *          |                                       |
       * 0x040000 +---------------------------------------+
       *          |  OTA                                  |
       *          |  464 KBytes                           |
       *          |                                       |
       *          |                                       |
       *          |                                       |
       * 0x0B4000 +---------------------------------------+
       *          |  File System                          |
       *          |                                       |
       *          |                                       |
       *          |                                       |
       *          |                                       |
       * 0x400000 +---------------------------------------+
       *
       */
      static constexpr size_t startAddress = 0x0B4000;
      static constexpr size_t size = 0x34C000;
      static constexpr size_t blockSize = 4096;

      bool resourcesValid = false;
      const struct lfs_config lfsConfig;

      lfs_t lfs;

      static int SectorSync(const struct lfs_config* c);
      static int SectorErase(const struct lfs_config* c, lfs_block_t block);
      static int SectorProg(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size);
      static int SectorRead(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size);
    };
  }
}
