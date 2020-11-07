#pragma once
#include <cstdint>
#include <array>
#include <host/ble_gap.h>

namespace Pinetime {
  namespace Controllers {
    class AlertNotificationService {
      public:
        AlertNotificationService(Pinetime::System::SystemTask &systemTask,
                                         Pinetime::Controllers::NotificationManager &notificationManager);
        void Init();

        int OnAlert(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt);


      private:
        static const char ALERT_UNKNOWN = '0x01';
        static const char ALERT_SIMPLE_ALERT = '0x02';
        static const char ALERT_EMAIL = '0x03';
        static const char ALERT_NEWS = '0x04';
        static const char ALERT_INCOMING_CALL = '0x05';
        static const char ALERT_MISSED_CALL = '0x06';
        static const char ALERT_SMS = '0x07';
        static const char ALERT_VOICE_MAIL = '0x08';
        static const char ALERT_SCHEDULE = '0x09';
        static const char ALERT_HIGH_PRIORITY_ALERT = '0x0a';
        static const char ALERT_INSTANT_MESSAGE = '0x0b';

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
