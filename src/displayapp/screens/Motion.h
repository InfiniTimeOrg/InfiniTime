#pragma once

#include <cstdint>
#include <chrono>
#include "displayapp/screens/Screen.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include "drivers/Bma421.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Motion : public Screen {
      public:
        explicit Motion(const Drivers::Bma421& motionSensor);
        ~Motion() override;

        void Refresh() override;

      private:
        const Drivers::Bma421& motionSensor;
        lv_obj_t* chart;
        lv_chart_series_t* ser1;
        lv_chart_series_t* ser2;
        lv_chart_series_t* ser3;
        lv_obj_t* label;

        lv_obj_t* labelStep;
        lv_task_t* taskRefresh;
      };
    }
  }
}
