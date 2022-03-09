#pragma once
#include <cstdint>
#include <lvgl/src/lv_misc/lv_color.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class BatteryIcon {
      public:
        static const char* GetUnknownIcon();
        static const char* GetBatteryIcon(uint8_t batteryPercent);
        static const lv_color_t GetBatteryColor(uint8_t batteryPercent);
        static const lv_color_t GetDefaultBatteryColor(uint8_t batteryPercent);
        static const char* GetPlugIcon(bool isCharging);
      };
    }
  }
}