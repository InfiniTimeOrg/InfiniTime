#include <cstdint>
#include "BatteryIcon.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace BatteryIcon {

        const char* GetBatteryIcon(uint8_t batteryPercent) {
          if (batteryPercent > 87)
            return Symbols::batteryFull;
          if (batteryPercent > 62)
            return Symbols::batteryThreeQuarter;
          if (batteryPercent > 37)
            return Symbols::batteryHalf;
          if (batteryPercent > 12)
            return Symbols::batteryOneQuarter;
          return Symbols::batteryEmpty;
        }

        const char* GetUnknownIcon() {
          return Symbols::batteryEmpty;
        }

        const char* GetPlugIcon(bool isCharging) {
          if (isCharging)
            return Symbols::plug;
          else
            return "";
        }

      }
    }
  }
}
