#pragma once

#include <cstdint>
#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Controllers {
    class Battery;
  }

  namespace Applications {
    namespace Screens {

      class BatteryInfo : public Screen {
      public:
        BatteryInfo(const Pinetime::Controllers::Battery& batteryController);
        ~BatteryInfo() override;

        void Refresh() override;

      private:
        const Pinetime::Controllers::Battery& batteryController;

        lv_obj_t* voltage;
        lv_obj_t* percent;
        lv_obj_t* charging_arc;
        lv_obj_t* animArc;
        lv_obj_t* charging_bolt;
        lv_obj_t* charging_bolt_bg;
        lv_obj_t* status;
        lv_style_t btn_style;

        lv_task_t* taskRefresh;

        uint8_t batteryPercent = 0;
        uint16_t batteryVoltage = 0;
      };
    }
  }
}
