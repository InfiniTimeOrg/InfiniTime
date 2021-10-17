#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include "components/fs/FS.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class Ble;
    class FSService {
    public:
      FSService(Pinetime::Controllers::FS& fs);
      void Init();

      int OnFSServiceRequested(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      void NotifyFSRaw(uint16_t connectionHandle);

    private:
      Pinetime::Controllers::FS& fs;
      static constexpr uint16_t FSServiceId {0xFEBB};
      static constexpr uint16_t fsVersionId {0x0100};
      static constexpr uint16_t fsTransferId {0x0200};
      uint16_t fsVersion = {0x0004};

      static constexpr ble_uuid128_t fsServiceUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0x72, 0x65, 0x66, 0x73, 0x6e, 0x61, 0x72, 0x54, 0x65, 0x6c, 0x69, 0x46, 0xBB, 0xFE, 0xAF, 0xAD}};

      static constexpr ble_uuid128_t fsVersionUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0x72, 0x65, 0x66, 0x73, 0x6e, 0x61, 0x72, 0x54, 0x65, 0x6c, 0x69, 0x46, 0x00, 0x01, 0xAF, 0xAD}};

      static constexpr ble_uuid128_t fsTransferUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0x72, 0x65, 0x66, 0x73, 0x6e, 0x61, 0x72, 0x54, 0x65, 0x6c, 0x69, 0x46, 0x00, 0x02, 0xAF, 0xAD}};

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];
      uint16_t versionCharacteristicHandle;

      enum class commands {
        INVALID = 0x00,
        READ = 0x10,
        READ_DATA = 0x11,
        READ_PACING = 0x12,
        WRITE = 0x20,
        WRITE_PACING = 0x21,
        WRITE_DATA = 0x22,
        DELETE = 0x30,
        DELETE_STATUS = 0x31,
        MKDIR = 0x40,
        MKDIR_STATUS = 0x41,
        LISTDIR = 0x50,
        LISTDIR_ENTRY = 0x51,
        MOVE = 0x60,
        MOVE_STATUS = 0x61,
      }
    };
  }
}
