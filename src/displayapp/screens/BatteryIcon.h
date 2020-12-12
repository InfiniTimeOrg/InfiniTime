#pragma once

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class BatteryIcon {
      public:
        static const char* GetUnknownIcon();
          static const char* GetBatteryIcon(float batteryPercent);
          static const char* GetPlugIcon(bool isCharging);
      };
    }
  }
}