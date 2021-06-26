#pragma once

#include <cstdint>
#include "drivers/SpiNorFlash.h"
#include <littlefs/lfs.h>

namespace Pinetime {
  namespace Controllers {
    class FS {
        public:
            FS(Pinetime::Drivers::SpiNorFlash&);

            Pinetime::Drivers::SpiNorFlash& mDriver;

            void Init();
            void LVGLFileSystemInit();

            int FileOpen(lfs_file_t* file_p, const char* fileName, const int flags);
            int FileClose(lfs_file_t* file_p);            
            int FileRead(lfs_file_t* file_p, uint8_t* buff, uint32_t size);
            int FileWrite(lfs_file_t* file_p, const uint8_t* buff, uint32_t size);
            int FileSeek(lfs_file_t* file_p, uint32_t pos);

            int FileDelete(const char* fileName);

            int DirCreate(const char* path);
            int DirDelete(const char* path);

            void VerifyResource();

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
            static constexpr size_t mStartAddress = 0x0B4000;
            static constexpr size_t mSize = 0x3C0000;

        private:
            bool resourcesValid = false;
            const struct lfs_config mLfsConfig;

            lfs_t mLfs;

    };
  }
}
