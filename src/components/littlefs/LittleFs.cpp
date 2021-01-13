#include "LittleFs.h"

using namespace Pinetime::System;

static int read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size);

static int prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size);

static int erase(const struct lfs_config *c, lfs_block_t block);

static int sync(const struct lfs_config *c);

constexpr size_t BLOCK_SIZE_BYTES = 4096u;
constexpr size_t PAGE_SIZE_BYTES = 256u;
constexpr size_t CACHE_PAGE_COUNT = 1u;
constexpr size_t CACHE_SIZE_BYTES = (CACHE_PAGE_COUNT * PAGE_SIZE_BYTES);
constexpr size_t LOOKAHEAD_SIZE_BYTES = (CACHE_PAGE_COUNT * 8u);
static uint8_t readBuffer[CACHE_SIZE_BYTES];
static uint8_t progBuffer[CACHE_SIZE_BYTES];
static uint8_t __attribute__((aligned(32)))
    lookaheadBuffer[LOOKAHEAD_SIZE_BYTES];
const static struct lfs_config baseLfsConfig = {
    .read = read,
    .prog = prog,
    .erase = erase,
    .sync = sync,

    .read_size = PAGE_SIZE_BYTES,
    .prog_size = PAGE_SIZE_BYTES,
    .block_size = BLOCK_SIZE_BYTES,
    .block_cycles = 500u,

    .cache_size = CACHE_SIZE_BYTES,
    .lookahead_size = LOOKAHEAD_SIZE_BYTES,

    .read_buffer = readBuffer,
    .prog_buffer = progBuffer,
    .lookahead_buffer = lookaheadBuffer,

    .name_max = 0u, /** use LFS default */
    .file_max = 0u, /** use LFS default */
    .attr_max = 0u, /** use LFS default */
};

constexpr struct lfs_config createLfsConfig(Pinetime::System::LittleFs& lfs, const size_t totalSize_bytes) {
    struct lfs_config config = baseLfsConfig;
    config.context = &lfs;
    config.block_count = totalSize_bytes / BLOCK_SIZE_BYTES;
    return config;
}

LittleFs::LittleFs(Pinetime::Drivers::SpiNorFlash& driver,
         const size_t startAddress,
         const size_t size_bytes,
         const bool allowFormat)
:mDriver{driver}, mStartAddress{startAddress}, mSize_bytes{size_bytes},
 mLfsConfig{createLfsConfig(*this, size_bytes)}
{
    // try mount
    if(0u != lfs_mount(&mLfs, &mLfsConfig)) {
        if(allowFormat) {
            // format and mount
            if(0u == lfs_format(&mLfs, &mLfsConfig)) {
                lfs_mount(&mLfs, &mLfsConfig);
            }
        }
    }
}

static int read(const struct lfs_config *c, lfs_block_t block,
                lfs_off_t off, void *buffer, lfs_size_t size) {
    Pinetime::System::LittleFs& lfs = *(reinterpret_cast<Pinetime::System::LittleFs*>(c->context));
    const size_t address = lfs.mStartAddress + (block * BLOCK_SIZE_BYTES) + off;
    lfs.mDriver.Read(address, (uint8_t*)buffer, size);
    // TODO assumes READ was successful
    return 0u;
}

static int prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    Pinetime::System::LittleFs& lfs = *(reinterpret_cast<Pinetime::System::LittleFs*>(c->context));
    const size_t address = lfs.mStartAddress + (block * BLOCK_SIZE_BYTES) + off;
    lfs.mDriver.Write(address, (uint8_t*)buffer, size);
    return lfs.mDriver.ProgramFailed() ? -1u : 0u;
}

static int erase(const struct lfs_config *c, lfs_block_t block) {
    Pinetime::System::LittleFs& lfs = *(reinterpret_cast<Pinetime::System::LittleFs*>(c->context));
    const size_t address = lfs.mStartAddress + (block * BLOCK_SIZE_BYTES);
    lfs.mDriver.SectorErase(address);
    return lfs.mDriver.EraseFailed() ? -1u : 0u;
}

static int sync(const struct lfs_config *c) {
    // no hardware caching used
    return 0u;
}

const LittleFs::LittleFsDir::LittleFsEntry LittleFs::LittleFsDir::next() {
    lfs_info entryInfo;
    const int rtn = lfs_dir_read(&mLfs, &mLfs_dir, &entryInfo);
    if(rtn == 0) {
        LittleFsEntry entry{
            .type = (entryInfo.type == LFS_TYPE_DIR) ?
                        LittleFs::LittleFsDir::LittleFsEntry::Type::DIR :
                        LittleFs::LittleFsDir::LittleFsEntry::Type::FILE,
            .size = entryInfo.size
        };
        strncpy(entry.name, entryInfo.name, sizeof(entry.name));
        return entry;
    }
    else
        return LittleFsEntry{ .type = LittleFs::LittleFsDir::LittleFsEntry::Type::NULLENTRY };
}