#pragma once

#include <cstdint>
#include <string>
#include <array>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#undef max
#undef min

//00030000-78fc-48fe-8e23-433b3a1942d0
#define QR_SERVICE_UUID_BASE {0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, 0x00, 0x00, 0x03, 0x00}

#define MAXLISTITEMS 4

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class QrService {
    public:
      struct QrInfo {
        std::string name;
        std::string text;
        bool operator==(const QrInfo &rhs) const {
            return (rhs.name == name && rhs.text == text);
        }
      };
      
      explicit QrService(Pinetime::System::SystemTask &system);

      void Init();

      int OnCommand(uint16_t conn_handle, uint16_t attr_handle,
                      struct ble_gatt_access_ctxt *ctxt);

      std::array<QrInfo, 4> getQrList();      

    private:

      static constexpr uint8_t qrsTextCharId[2] = {0x00, 0x01};

      ble_uuid128_t qrsUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = QR_SERVICE_UUID_BASE
      };

      ble_uuid128_t qrsTextCharUuid{
          .u = {.type = BLE_UUID_TYPE_128},
          .value = QR_SERVICE_UUID_BASE
      };

      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];

      std::array<QrInfo, 4> qrList;

      Pinetime::System::SystemTask &m_system;

    };
  }
}
