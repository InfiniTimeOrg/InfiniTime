#pragma once

#include <cstdint>
#include <FreeRTOS.h>
#include <timers.h>
#include "Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Controllers {
    class Battery;
  }

  namespace Applications {
    namespace Screens {

      class BatteryInfo : public Screen {
      public:
        BatteryInfo(DisplayApp* app, Pinetime::Controllers::Battery& batteryController);
        ~BatteryInfo() override;

        bool Refresh() override;

        void UpdateScreen();
        void UpdateAnim();

      private:
        Pinetime::Controllers::Battery& batteryController;

        lv_obj_t* voltage;
        lv_obj_t* percent;
        lv_obj_t* charging_bar;
        lv_obj_t* status;

        lv_task_t* taskUpdate;
        lv_task_t* taskAnim;

        int8_t animation = 0;
        int8_t batteryPercent = -1;
        uint16_t batteryVoltage = 0;
      };
    }
  }
}
