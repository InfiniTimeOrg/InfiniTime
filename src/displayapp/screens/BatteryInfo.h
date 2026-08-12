#pragma once

#include <cstdint>
#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "utility/DirtyValue.h"

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
        lv_obj_t* chargingArc;
        lv_obj_t* status;

        lv_task_t* taskRefresh;

        Utility::DirtyValue<uint8_t> batteryPercent{};
        Utility::DirtyValue<uint16_t> batteryVoltage{};
        Utility::DirtyValue<bool> batteryCharging{};
      };
    }
  }
}
