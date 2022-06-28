#pragma once
#include <cstdint>
#include <lvgl/src/lv_core/lv_obj.h>
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class BatteryIcon {
      public:
        void Create(lv_obj_t* parent);

        void SetColor(lv_color_t);
        void SetBatteryPercentage(uint8_t percentage, bool show_percentage);
        lv_obj_t* GetObject();

        static const char* GetUnknownIcon();
        static const char* GetPlugIcon(bool isCharging);

      private:
        lv_obj_t* batteryContainer;
        lv_obj_t* batteryImg;
        lv_obj_t* batteryJuice;
        lv_obj_t* batteryPercentageText;
      };
    }
  }
}
