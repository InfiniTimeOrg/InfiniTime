#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include "Version.h"

// Service UUID
// 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
#define NUS_SERVICE_UUID_BASE \
  { 0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e  }

//6E400002-B5A3-F393-E0A9-E50E24DCCA9E
#define NUS_CHARACTERTISTIC_RX_UUID \
  { 0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e  }

//6E400003-B5A3-F393-E0A9-E50E24DCCA9E
#define NUS_CHARACTERTISTIC_TX_UUID \
  { 0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e  }


namespace Pinetime {
  namespace Controllers {
    class BleNus {
    public:
      BleNus();
      void Init();
      void SetConnectionHandle(uint16_t conn_handle);
      void Print(char *str);


      int OnDeviceInfoRequested(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt);

    private:

      static uint16_t attr_read_handle;
      uint16_t conn_handle;

      static constexpr ble_uuid128_t nusServiceUuid {.u {.type = BLE_UUID_TYPE_128}, .value = NUS_SERVICE_UUID_BASE};

      static constexpr ble_uuid128_t rxCharacteristicUuid {.u {.type = BLE_UUID_TYPE_128}, .value = NUS_CHARACTERTISTIC_RX_UUID};

      static constexpr ble_uuid128_t txCharacteristicUuid {.u {.type = BLE_UUID_TYPE_128}, .value = NUS_CHARACTERTISTIC_TX_UUID};

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];
    };
  }
}