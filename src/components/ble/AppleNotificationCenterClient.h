#pragma once

#include <cstdint>
#include <functional>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include "components/ble/BleClient.h"

namespace Pinetime {

  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class NotificationManager;

    class AppleNotificationCenterClient : public BleClient {
    public:
      explicit AppleNotificationCenterClient(Pinetime::System::SystemTask& systemTask,
                                       Pinetime::Controllers::NotificationManager& notificationManager);

      bool OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_svc* service);
      int OnCharacteristicsDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_chr* characteristic);
      int OnNewAlertSubcribe(uint16_t connectionHandle, const ble_gatt_error* error, ble_gatt_attr* attribute);
      int OnDescriptorDiscoveryEventCallback(uint16_t connectionHandle, const ble_gatt_error* error, uint16_t characteristicValueHandle, const ble_gatt_dsc* descriptor);
      void OnNotification(ble_gap_event* event);
      void Reset();
      void Discover(uint16_t connectionHandle, std::function<void(uint16_t)> lambda) override;

    private:
      // 7905F431-B5CE-4E99-A40F-4B1E122D00D0
      const ble_uuid128_t ancsUuid { 
        {BLE_UUID_TYPE_128},
        {0xd0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0x24, 0x99, 0x0E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79}
      };

      // 9FBF120D-6301-42D9-8C58-25E699A21DBD
      const ble_uuid128_t notificationSourceChar { 
        {BLE_UUID_TYPE_128}, 
        {0xBD, 0x1D, 0xA2, 0x99, 0xE6, 0x25, 0x58, 0X0C, 0xD9, 0x02, 0x01, 0x63, 0x0D, 0x12, 0xBF, 0x9F}
      };
      // 69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9
      const ble_uuid128_t controlPointChar {
        {BLE_UUID_TYPE_128},
        {0xD9, 0xD9, 0xAA, 0xFD, 0xBD, 0x9B, 0x21, 0X18,0xA8, 0x09,0xE1, 0x45, 0xF3, 0xD8, 0xD1, 0x69 }
      };
      // 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB
      const ble_uuid128_t dataSourceChar {
        {BLE_UUID_TYPE_128},
        {0xFB, 0x7B, 0x7C, 0xCE, 0x6A, 0xB3, 0x44, 0X3E,0xB5, 0x0B,0xD6, 0x24, 0xE9, 0xC6, 0xEA, 0x22 }
      };

      const ble_uuid16_t gattServiceUuid = {BLE_UUID_TYPE_16, 0x1801};
      const ble_uuid16_t serviceChangedCharUuid = {BLE_UUID_TYPE_16, 0x2A05};


      uint16_t ancsStartHandle {0};
      uint16_t ancsEndHandle {0};
      uint16_t notificationSourceHandle {0};
      //uint16_t controlPointHandle {0};
      //uint16_t dataSourceHandle {0};
      uint16_t notificationSourceDescriptorHandle {0};
      uint16_t serviceChangedHandle {0};
      bool isDiscovered {false};
      bool isCharacteristicDiscovered {false};
      bool isDescriptorFound {false};
      Pinetime::System::SystemTask& systemTask;
      Pinetime::Controllers::NotificationManager& notificationManager;
      std::function<void(uint16_t)> onServiceDiscovered;
    };
  }
}