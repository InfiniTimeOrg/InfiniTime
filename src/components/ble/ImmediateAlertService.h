#pragma once
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

    class ImmediateAlertService {
    public:
      enum class Levels : uint8_t { NoAlert = 0, MildAlert = 1, HighAlert = 2 };

      ImmediateAlertService(Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::NotificationManager& notificationManager);
      void Init();
      int OnAlertLevelChanged(uint16_t attributeHandle, ble_gatt_access_ctxt* context);

    private:
      Pinetime::System::SystemTask& systemTask;
      NotificationManager& notificationManager;

      static constexpr uint16_t immediateAlertServiceId {0x1802};
      static constexpr uint16_t alertLevelId {0x2A06};

      static constexpr ble_uuid16_t immediateAlertServiceUuid {.u {.type = BLE_UUID_TYPE_16}, .value = immediateAlertServiceId};

      static constexpr ble_uuid16_t alertLevelUuid {.u {.type = BLE_UUID_TYPE_16}, .value = alertLevelId};

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t alertLevelHandle;
    };
  }
}
