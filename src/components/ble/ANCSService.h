#pragma once
#include <cstdint>
#include <array>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

// Primary ANCS UUID: 7905F431-B5CE-4E99-A40F-4B1E122D00D0
#define ANCS_SERVICE_UUID_BASE { 0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79 }

// Notification Source UUID: 9FBF120D-6301-42D9-8C58-25E699A21DBD
#define ANCS_NOTIFICATION_SOURCE_UUID { 0xBD, 0x1D, 0xA2, 0x99, 0xE6, 0x25, 0x58, 0x8C, 0xD9, 0x42, 0x01, 0x63, 0x0D, 0x12, 0xBF, 0x9F }

namespace Pinetime {

  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class NotificationManager;

    class ANCSService {
    public:
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

      ANCSService(Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::NotificationManager& notificationManager);
      void Init();

      int OnAlert(struct ble_gatt_access_ctxt* ctxt);

    private:

      static constexpr ble_uuid128_t notificationSourceCharUuid {.u {.type = BLE_UUID_TYPE_128}, .value = ANCS_NOTIFICATION_SOURCE_UUID};

      static constexpr ble_uuid128_t ancsBaseUuid {.u {.type = BLE_UUID_TYPE_128}, .value = ANCS_SERVICE_UUID_BASE};

      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];

      Pinetime::System::SystemTask& systemTask;
      NotificationManager& notificationManager;

      uint16_t eventHandle;
    };
  }
}