#pragma once
#include <cstdint>
#include <array>
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

namespace Pinetime {

  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class NotificationManager;

    class AlertNotificationService {
      public:
        AlertNotificationService(Pinetime::System::SystemTask &systemTask,
                                         Pinetime::Controllers::NotificationManager &notificationManager);
        void Init();

        int OnAlert(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt);


      private:
        static constexpr uint16_t ansId {0x1811};
        static constexpr uint16_t ansCharId {0x2a46};

        static constexpr ble_uuid16_t ansUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = ansId
        };

        static constexpr ble_uuid16_t ansCharUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = ansCharId
        };

        struct ble_gatt_chr_def characteristicDefinition[2];
        struct ble_gatt_svc_def serviceDefinition[2];

        Pinetime::System::SystemTask &systemTask;
        NotificationManager &notificationManager;
    };
  }
}
