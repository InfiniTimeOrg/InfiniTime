#pragma once

#include <cstdint>
#include <array>

#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

#include <LittleFs.h>

namespace Pinetime {
  namespace System {
    class SystemTask;
    class LittleFs;
  }
  namespace Controllers {
    class Ble;

    /**
     * Implements [Bluetooth FTP](https://www.bluetooth.org/docman/handlers/DownloadDoc.ashx?doc_id=309003)
     * over GATT.
     */
    class FtpService {
      public:
        FtpService(Pinetime::System::SystemTask &systemTask,
                   Pinetime::Controllers::Ble &bleController,
                   Pinetime::System::LittleFs &littleFs);
        void Init();
        int OnServiceData(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt *context);

      private:
        static constexpr ble_uuid128_t serviceUuid{
                .u {.type = BLE_UUID_TYPE_128},
                .value = {0xF9, 0xEC, 0x7B, 0xC4, 0x95, 0x3C, 0x11, 0xD2,
                          0x98, 0x4E, 0x52, 0x54, 0x00, 0xDC, 0x9E, 0x09}
        };

        static constexpr uint16_t revision{0x0001};
        static constexpr ble_uuid128_t revisionCharacteristicUuid{
                .u {.type = BLE_UUID_TYPE_128},
                .value = {0xF9, 0xEC, 0x7B, 0xC4, 0x95, 0x3C, 0x11, 0xD2,
                          0x98, 0x4E, 0x52, 0x54, 0x34, 0x15, 0x00, 0x00}
        };

        static constexpr ble_uuid128_t pathCharacteristicUuid{
                .u {.type = BLE_UUID_TYPE_128},
                .value = {0xF9, 0xEC, 0x7B, 0xC4, 0x95, 0x3C, 0x11, 0xD2,
                          0x98, 0x4E, 0x52, 0x54, 0x30, 0x15, 0x00, 0x00}
        };

        static constexpr ble_uuid128_t controlPointCharacteristicUuid{
                .u {.type = BLE_UUID_TYPE_128},
                .value = {0xF9, 0xEC, 0x7B, 0xC4, 0x95, 0x3C, 0x11, 0xD2,
                          0x98, 0x4E, 0x52, 0x54, 0x31, 0x15, 0x00, 0x00}
        };

        static constexpr ble_uuid128_t packetCharacteristicUuid{
                .u {.type = BLE_UUID_TYPE_128},
                .value = {0xF9, 0xEC, 0x7B, 0xC4, 0x95, 0x3C, 0x11, 0xD2,
                          0x98, 0x4E, 0x52, 0x54, 0x32, 0x15, 0x00, 0x00}
        };

        Pinetime::System::SystemTask &mSystemTask;
        Pinetime::Controllers::Ble &mBleController;
        Pinetime::System::LittleFs &mLittleFs;
        const struct ble_gatt_chr_def mCharacteristicDefinitions[5];    ///< number of characteristics (plus one null)
        const struct ble_gatt_svc_def mServiceDefinitions[2];           ///< number of services (plus one null)
        static constexpr size_t MAX_PATH_LENGTH = 512;
        char mCurrentPath[MAX_PATH_LENGTH] = "";
        size_t mCurrentPath_sz = 1;
        Pinetime::System::LittleFs::LittleFsFile* mFile;
        Pinetime::System::LittleFs::LittleFsDir* mDir;
        void closeActive();
        int handleWrite(ble_gatt_access_ctxt*);
        int handleRead(ble_gatt_access_ctxt*);

        enum class Opcodes : uint8_t {
            REMOVE = 0x82,      ///< delete file or empty directory
            MKDIR = 0x85,
            READ = 0x87,        /**< streams out file contents upon each read of
                                 * packet characteristic */
            WRITE = 0x88,       /**< will create new file if non-existent, appends
                                 * to trunced file upon each write of packet characteristic */
            CLOSE = 0x89,       ///< commit written data
            LIST = 0x80,        ///< get directory contents (x-obex/folderlisting)
        };
        enum class ResultCodes : uint8_t {
            SUCCESS = 0x00,
            FAILURE = 0xFF,
        };

        enum class ListingState : uint8_t {
            DONE,
            HDR_1,  ///< xml version string
            HDR_2,  ///< doctype
            HDR_3,  ///< <folder-listing> <parent-folder/>
            ENTRIES,
            FTR,    ///< </folder-listing>
        };
        ListingState mListingState = ListingState::DONE;
    };
  }
}