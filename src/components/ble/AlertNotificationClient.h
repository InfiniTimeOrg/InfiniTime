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

    class AlertNotificationClient : public BleClient {
    public:
      explicit AlertNotificationClient(Pinetime::System::SystemTask& systemTask,
                                       Pinetime::Controllers::NotificationManager& notificationManager);

      bool OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_svc* service);
      int OnCharacteristicsDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_chr* characteristic);
      int OnNewAlertSubcribe(uint16_t connectionHandle, const ble_gatt_error* error);
      int OnDescriptorDiscoveryEventCallback(uint16_t connectionHandle,
                                             const ble_gatt_error* error,
                                             uint16_t characteristicValueHandle,
                                             const ble_gatt_dsc* descriptor);
      void OnNotification(ble_gap_event* event);
      void Reset();
      void Discover(uint16_t connectionHandle, std::function<void(uint16_t)> lambda) override;

    private:
      static constexpr uint16_t ansServiceId {0x1811};
      static constexpr uint16_t supportedNewAlertCategoryId = 0x2a47;
      static constexpr uint16_t supportedUnreadAlertCategoryId = 0x2a48;
      static constexpr uint16_t newAlertId = 0x2a46;
      static constexpr uint16_t unreadAlertStatusId = 0x2a45;
      static constexpr uint16_t controlPointId = 0x2a44;

      static constexpr ble_uuid16_t ansServiceUuid {.u {.type = BLE_UUID_TYPE_16}, .value = ansServiceId};
      static constexpr ble_uuid16_t supportedNewAlertCategoryUuid {.u {.type = BLE_UUID_TYPE_16}, .value = supportedNewAlertCategoryId};
      static constexpr ble_uuid16_t supportedUnreadAlertCategoryUuid {.u {.type = BLE_UUID_TYPE_16},
                                                                      .value = supportedUnreadAlertCategoryId};
      static constexpr ble_uuid16_t newAlertUuid {.u {.type = BLE_UUID_TYPE_16}, .value = newAlertId};
      static constexpr ble_uuid16_t unreadAlertStatusUuid {.u {.type = BLE_UUID_TYPE_16}, .value = unreadAlertStatusId};
      static constexpr ble_uuid16_t controlPointUuid {.u {.type = BLE_UUID_TYPE_16}, .value = controlPointId};

      uint16_t ansStartHandle = 0;
      uint16_t ansEndHandle = 0;
      uint16_t supportedNewAlertCategoryHandle = 0;
      uint16_t supportedUnreadAlertCategoryHandle = 0;
      uint16_t newAlertHandle = 0;
      uint16_t newAlertDescriptorHandle = 0;
      uint16_t newAlertDefHandle = 0;
      uint16_t unreadAlertStatusHandle = 0;
      uint16_t controlPointHandle = 0;
      bool isDiscovered = false;
      Pinetime::System::SystemTask& systemTask;
      Pinetime::Controllers::NotificationManager& notificationManager;
      std::function<void(uint16_t)> onServiceDiscovered;
      bool isCharacteristicDiscovered = false;
      bool isDescriptorFound = false;
    };
  }
}
