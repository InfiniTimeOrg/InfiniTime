#pragma once

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace BatteryIcon {
        const char* GetUnknownIcon();
        const char* GetBatteryIcon(uint8_t batteryPercent);
        const char* GetPlugIcon(bool isCharging);
      }
    }
  }
}
