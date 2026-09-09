#pragma once

#include <cstdint>
#include "displayapp/screens/Screen.h"
#include "components/settings/Settings.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Controllers {
    class Battery;
  }

  namespace Applications {
    namespace Screens {

      class BatteryInfo : public Screen {
      public:
        BatteryInfo(const Pinetime::Controllers::Battery& batteryController, Pinetime::Controllers::Settings& settingsController);
        ~BatteryInfo() override;

        void Refresh() override;

      private:
        const Pinetime::Controllers::Battery& batteryController;
        Pinetime::Controllers::Settings& settingsController;

        lv_obj_t* voltage;
        lv_obj_t* percent;
        lv_obj_t* chargingArc;
        lv_obj_t* status;

        lv_task_t* taskRefresh;

        uint8_t batteryPercent = 0;
        uint16_t batteryVoltage = 0;
      };
    }
  }
}
