#pragma once

#include <drivers/SpiNorFlash.h>
#include <lfs.h>

namespace Pinetime {
    namespace System {
        class LittleFs {
            public:
                class LittleFsFile;
                class LittleFsDir;
                explicit LittleFs(Pinetime::Drivers::SpiNorFlash&,
                                  const size_t startAddress,    ///< address of first block to use
                                  const size_t size_bytes,      ///< must be a multiple of block size
                                  const bool allowFormat = true ///< if filesystem not detected, format the space
                                  );
                LittleFs(const LittleFs&) = delete;
                LittleFs& operator=(const LittleFs&) = delete;
                LittleFs(LittleFs&&) = delete;
                LittleFs& operator=(LittleFs&&) = delete;

                bool remove(const char* path) { return 0u == lfs_remove(&mLfs, path); }
                bool rename(const char* oldpath, const char* newpath) { return 0u == lfs_rename(&mLfs, oldpath, newpath); }
                ssize_t filesize(const char* path) {
                    struct lfs_info info;
                    if(0u == lfs_stat(&mLfs, path, &info))
                        return info.size;
                    return -1;
                }
                /// file resource is closed upon leave of scope
                LittleFsFile& open(const char* const path,
                                  const int flags           ///< @see lfs_open_flags
                                  ) { return *(new LittleFsFile(mLfs, path, flags)); }
                bool mkdir(const char* path) { return 0u == lfs_mkdir(&mLfs, path); }
                /// dir resource is closed upon leave of scope
                LittleFsDir& opendir(const char* path) { return *(new LittleFsDir(mLfs, path)); }

                // the following require owner to delete the instance
                LittleFsFile* open_danger(const char* const path,
                                  const int flags           ///< @see lfs_open_flags
                                  ) { return new LittleFsFile(mLfs, path, flags); }
                LittleFsDir* opendir_danger(const char* const path) { return new LittleFsDir(mLfs, path); }

                const size_t mStartAddress;
                const size_t mSize_bytes;
                Pinetime::Drivers::SpiNorFlash& mDriver;

            private:
                lfs_t mLfs;
                const struct lfs_config mLfsConfig;

            public:
                class LittleFsFile {
                    public:
                        explicit LittleFsFile(lfs_t& lfs, const char* const path, const int flags)
                            : mLfs{lfs} { lfs_file_open(&mLfs, &mLfs_file, path, flags); }
                        LittleFsFile(const LittleFsFile&) = delete;
                        LittleFsFile& operator=(const LittleFsFile&) = delete;
                        LittleFsFile(LittleFsFile&&) = delete;
                        LittleFsFile& operator=(LittleFsFile&&) = delete;
                        ~LittleFsFile() {
                            lfs_file_close(&mLfs, &mLfs_file);
                        }
                        bool isNULL() { return mLfs_file.flags != LFS_F_ERRED; }
                        ssize_t read(void* buffer, size_t size) { return lfs_file_read(&mLfs, &mLfs_file, buffer, size); }
                        ssize_t write(const void* buffer, size_t size) { return lfs_file_write(&mLfs, &mLfs_file, buffer, size); }
                        ssize_t seek(size_t offset, size_t whence) { return 0u == lfs_file_seek(&mLfs, &mLfs_file, offset, whence); }
                        bool truncate(size_t sz) { return 0u == lfs_file_truncate(&mLfs, &mLfs_file, sz); }
                        /// return the current position within the file
                        ssize_t tell() { return lfs_file_tell(&mLfs, &mLfs_file); }
                        ssize_t size() { return lfs_file_size(&mLfs, &mLfs_file); }
                    private:
                        lfs_t& mLfs;
                        lfs_file_t mLfs_file;
                };

                class LittleFsDir {
                    public:
                        explicit LittleFsDir(lfs_t& lfs, const char* path)
                            : mLfs{lfs} { mLfs_dir.type = -1; lfs_dir_open(&mLfs, &mLfs_dir, path); }
                        LittleFsDir(const LittleFsDir&) = delete;
                        LittleFsDir& operator=(const LittleFsDir&) = delete;
                        LittleFsDir(LittleFsDir&&) = delete;
                        LittleFsDir& operator=(LittleFsDir&&) = delete;
                        ~LittleFsDir() {
                            lfs_dir_close(&mLfs, &mLfs_dir);
                        }
                        bool isNULL() { return mLfs_dir.type != LFS_TYPE_DIR; }
                        // TODO determine API for directories
                    private:
                        lfs_t& mLfs;
                        lfs_dir_t mLfs_dir;
                };
        };
    }
}
