#pragma once

#include <cstdint>
#include <functional>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include "components/ble/BleClient.h"
#include <unordered_map>
#include <string>

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
      int OnDescriptorDiscoveryEventCallback(uint16_t connectionHandle,
                                             const ble_gatt_error* error,
                                             uint16_t characteristicValueHandle,
                                             const ble_gatt_dsc* descriptor);
      int OnControlPointWrite(uint16_t connectionHandle, const ble_gatt_error* error, ble_gatt_attr* attribute);
      void MaybeFinishDiscovery(uint16_t connectionHandle);
      void OnNotification(ble_gap_event* event);
      void Reset();
      void Discover(uint16_t connectionHandle, std::function<void(uint16_t)> lambda) override;
      void DebugNotification(const char* msg) const;

      void AcceptIncomingCall(uint32_t notificationUid);
      void RejectIncomingCall(uint32_t notificationUid);

      static constexpr uint8_t maxTitleSize {20};
      static constexpr uint8_t maxSubtitleSize {15};
      static constexpr uint8_t maxMessageSize {120};

      // The Apple Notification Center Service UUID are from
      // https://developer.apple.com/library/archive/documentation/CoreBluetooth/Reference/AppleNotificationCenterServiceSpecification/Specification/Specification.html

      // 7905F431-B5CE-4E99-A40F-4B1E122D00D0
      static constexpr ble_uuid128_t ancsUuid {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0xd0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79}};

    private:
      // 9FBF120D-6301-42D9-8C58-25E699A21DBD
      static constexpr ble_uuid128_t notificationSourceChar {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0xBD, 0x1D, 0xA2, 0x99, 0xE6, 0x25, 0x58, 0x8C, 0xD9, 0x42, 0x01, 0x63, 0x0D, 0x12, 0xBF, 0x9F}};
      // 69D1D8F3-45E1-49A8-9821-9BBDFDAAD9D9
      static constexpr ble_uuid128_t controlPointChar {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0xD9, 0xD9, 0xAA, 0xFD, 0xBD, 0x9B, 0x21, 0x98, 0xA8, 0x49, 0xE1, 0x45, 0xF3, 0xD8, 0xD1, 0x69}};
      // 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB
      static constexpr ble_uuid128_t dataSourceChar {
        .u {.type = BLE_UUID_TYPE_128},
        .value = {0xFB, 0x7B, 0x7C, 0xCE, 0x6A, 0xB3, 0x44, 0xBE, 0xB5, 0x4B, 0xD6, 0x24, 0xE9, 0xC6, 0xEA, 0x22}};

      static constexpr ble_uuid16_t gattServiceUuid = {BLE_UUID_TYPE_16, 0x1801};
      static constexpr ble_uuid16_t serviceChangedCharUuid = {BLE_UUID_TYPE_16, 0x2A05};

      enum class Categories : uint8_t {
        Other = 0,
        IncomingCall = 1,
        MissedCall = 2,
        Voicemail = 3,
        Social = 4,
        Schedule = 5,
        Email = 6,
        News = 7,
        HealthAndFitness = 8,
        BuissnessAndFinance = 9,
        Location = 10,
        Entertainment = 11
      };

      enum class EventIds : uint8_t { Added = 0, Modified = 1, Removed = 2 };

      enum class EventFlags : uint8_t {
        Silent = (1 << 0),
        Important = (1 << 1),
        PreExisting = (1 << 2),
        PositiveAction = (1 << 3),
        NegativeAction = (1 << 4)
      };

      struct AncsNotification {
        uint8_t eventId {0};
        uint8_t eventFlags {0};
        uint8_t category {0};
        uint32_t uuid {0};
        bool isProcessed {false};
      };

      std::unordered_map<uint32_t, AncsNotification> notifications;

      std::string DecodeUtf8String(os_mbuf* om, uint16_t size, uint16_t offset);

      bool subscriptionsDone = false;
      uint16_t ancsStartHandle {0};
      uint16_t ancsEndHandle {0};
      uint16_t notificationSourceHandle {0};
      uint16_t controlPointHandle {0};
      uint16_t dataSourceHandle {0};
      uint16_t notificationSourceDescriptorHandle {0};
      uint16_t controlPointDescriptorHandle {0};
      uint16_t dataSourceDescriptorHandle {0};

      uint16_t gattStartHandle {0};
      uint16_t gattEndHandle {0};
      uint16_t serviceChangedHandle {0};
      uint16_t serviceChangedDescriptorHandle {0};
      bool isGattDiscovered {false};
      bool isGattCharacteristicDiscovered {false};
      bool isGattDescriptorFound {false};
      bool isDiscovered {false};
      bool isCharacteristicDiscovered {false};
      bool isDescriptorFound {false};
      bool isControlCharacteristicDiscovered {false};
      bool isControlDescriptorFound {false};
      bool isDataCharacteristicDiscovered {false};
      bool isDataDescriptorFound {false};
      Pinetime::System::SystemTask& systemTask;
      Pinetime::Controllers::NotificationManager& notificationManager;
      std::function<void(uint16_t)> onServiceDiscovered;
    };
  }
}
