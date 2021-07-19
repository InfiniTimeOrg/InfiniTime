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

      private:
        Pinetime::Controllers::Battery& batteryController;

        lv_obj_t* voltage;
        lv_obj_t* percent;
        lv_obj_t* charging_bar;
        lv_obj_t* status;

        lv_task_t* taskUpdate;

        uint8_t batteryPercent = 0;
        uint16_t batteryVoltage = 0;
      };
    }
  }
}
