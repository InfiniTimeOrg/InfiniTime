#pragma once
#include <cstdint>
#include <array>
#include <host/ble_gap.h>


namespace Pinetime {
  namespace Controllers {
    int AlertNotificationDiscoveryEventCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                                                       const struct ble_gatt_svc *service, void *arg);
    int AlertNotificationCharacteristicsDiscoveryEventCallback(uint16_t conn_handle,
                                                              const struct ble_gatt_error *error,
                                                              const struct ble_gatt_chr *chr, void *arg);
    int NewAlertSubcribeCallback(uint16_t conn_handle,
                                                        const struct ble_gatt_error *error,
                                                        struct ble_gatt_attr *attr,
                                                        void *arg);

    int AlertNotificationDescriptorDiscoveryEventCallback(uint16_t conn_handle,
                                                          const struct ble_gatt_error *error,
                                                          uint16_t chr_val_handle,
                                                          const struct ble_gatt_dsc *dsc,
                                                          void *arg);

    class AlertNotificationClient {
      public:
        explicit AlertNotificationClient(Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::NotificationManager& notificationManager);
        void Init();


        void StartDiscovery(uint16_t connectionHandle);
        bool OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error, const ble_gatt_svc *service);
        int OnCharacteristicsDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error,
                                            const ble_gatt_chr *characteristic);
        int OnNewAlertSubcribe(uint16_t connectionHandle, const ble_gatt_error *error, ble_gatt_attr *attribute);
        int OnDescriptorDiscoveryEventCallback(uint16_t connectionHandle, const ble_gatt_error *error,
                                               uint16_t characteristicValueHandle, const ble_gatt_dsc *descriptor);
        void OnNotification(ble_gap_event *event);
      private:
        static constexpr uint16_t ansServiceId {0x1811};
        static constexpr uint16_t supportedNewAlertCategoryId = 0x2a47;
        static constexpr uint16_t supportedUnreadAlertCategoryId = 0x2a48;
        static constexpr uint16_t newAlertId = 0x2a46;
        static constexpr uint16_t unreadAlertStatusId = 0x2a45;
        static constexpr uint16_t controlPointId = 0x2a44;

        static constexpr ble_uuid16_t ansServiceUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = ansServiceId
        };
        static constexpr ble_uuid16_t supportedNewAlertCategoryUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = supportedNewAlertCategoryId
        };
        static constexpr ble_uuid16_t supportedUnreadAlertCategoryUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = supportedUnreadAlertCategoryId
        };
        static constexpr ble_uuid16_t newAlertUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = newAlertId
        };
        static constexpr ble_uuid16_t unreadAlertStatusUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = unreadAlertStatusId
        };
        static constexpr ble_uuid16_t controlPointUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = controlPointId
        };

        uint16_t ansEndHandle;
        uint16_t supportedNewAlertCategoryHandle;
        uint16_t supportedUnreadAlertCategoryHandle;
        uint16_t newAlertHandle;
        uint16_t newAlertDescriptorHandle = 0;
        uint16_t newAlertDefHandle;
        uint16_t unreadAlertStatusHandle;
        uint16_t controlPointHandle;
        bool discoveryDone = false;
        Pinetime::System::SystemTask& systemTask;
        Pinetime::Controllers::NotificationManager& notificationManager;

    };


  }
}