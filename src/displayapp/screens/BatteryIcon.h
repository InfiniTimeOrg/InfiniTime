#pragma once
#include <cstdint>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class BatteryIcon {
      public:
        static const char* GetUnknownIcon();
        static const char* GetBatteryIcon(uint8_t batteryPercent);
        static const char* GetPlugIcon(bool isCharging);
      };
    }
  }
}