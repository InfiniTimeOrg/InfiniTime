#pragma once
#include <cstdint>
#include <lvgl/src/lv_core/lv_obj.h>
#include "displayapp/Colors.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class BatteryIcon {
      public:
        explicit BatteryIcon(bool colorOnLowBattery);
        void Create(lv_obj_t* parent);

        void SetColor(Colors::Color);
        void SetBatteryPercentage(uint8_t percentage);
        lv_obj_t* GetObject();

        static const char* GetUnknownIcon();
        static const char* GetPlugIcon(bool isCharging);

      private:
        lv_obj_t* batteryImg;
        lv_obj_t* batteryJuice;
        bool colorOnLowBattery = false;
      };
    }
  }
}
