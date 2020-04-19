#pragma once

#include <cstdint>
#include <host/ble_gap.h>

namespace Pinetime {
  namespace Controllers {

    class NimbleController {
      public:
        void Init();
        void StartAdvertising();
        int OnGAPEvent(ble_gap_event *event);
      private:
        static constexpr char* deviceName = "Pinetime-JF";
        uint8_t addrType;
    };
  }
}
