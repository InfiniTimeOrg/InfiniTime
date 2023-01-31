#pragma once
#include <cstdint>
#include <array>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

// 00020001-78fc-48fe-8e23-433b3a1942d0
#define NOTIFICATION_EVENT_SERVICE_UUID_BASE                                                                                               \
  { 0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, 0x01, 0x00, 0x02, 0x00 }

namespace Pinetime {

  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class NotificationManager;

    class AlertNotificationService {
    public:
      AlertNotificationService(Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::NotificationManager& notificationManager);
      void Init();

      int OnAlert(struct ble_gatt_access_ctxt* ctxt);

      void AcceptIncomingCall();
      void RejectIncomingCall();
      void MuteIncomingCall();

      enum class IncomingCallResponses : uint8_t { Reject = 0x00, Answer = 0x01, Mute = 0x02 };

    private:
      enum class Categories : uint8_t {
        SimpleAlert = 0x00,
        Email = 0x01,
        News = 0x02,
        Call = 0x03,
        MissedCall = 0x04,
        MmsSms = 0x05,
        VoiceMail = 0x06,
        Schedule = 0x07,
        HighPrioritizedAlert = 0x08,
        InstantMessage = 0x09,
        All = 0xff
      };

      static constexpr uint16_t ansId {0x1811};
      static constexpr uint16_t ansCharId {0x2a46};

      static constexpr ble_uuid16_t ansUuid {.u {.type = BLE_UUID_TYPE_16}, .value = ansId};

      static constexpr ble_uuid16_t ansCharUuid {.u {.type = BLE_UUID_TYPE_16}, .value = ansCharId};

      static constexpr ble_uuid128_t notificationEventUuid {.u {.type = BLE_UUID_TYPE_128}, .value = NOTIFICATION_EVENT_SERVICE_UUID_BASE};

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];

      Pinetime::System::SystemTask& systemTask;
      NotificationManager& notificationManager;

      uint16_t eventHandle;
    };
  }
}
