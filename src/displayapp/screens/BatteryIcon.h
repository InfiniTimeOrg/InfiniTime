#pragma once

#include <libs/lvgl/src/lv_draw/lv_img_decoder.h>

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