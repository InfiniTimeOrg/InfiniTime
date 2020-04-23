#pragma once

#include <cstdint>

#include "DeviceInformationService.h"
#include "CurrentTimeClient.h"
#include <host/ble_gap.h>

namespace Pinetime {
  namespace Controllers {
    class DateTime;
    class NimbleController {
      public:
        NimbleController(DateTime& dateTimeController);
        void Init();
        void StartAdvertising();
        int OnGAPEvent(ble_gap_event *event);

      private:
        static constexpr char* deviceName = "Pinetime-JF";
        DateTime& dateTimeController;
        DeviceInformationService deviceInformationService;
        CurrentTimeClient currentTimeClient;
        uint8_t addrType;
        uint16_t connectionHandle;
    };
  }
}
