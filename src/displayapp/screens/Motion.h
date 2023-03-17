#pragma once

#include <cstdint>
#include <chrono>
#include "displayapp/screens/Screen.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <components/motion/MotionController.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Motion : public Screen {
      public:
        Motion(Controllers::MotionController& motionController);
        ~Motion() override;

        void Refresh() override;

      private:
        Controllers::MotionController& motionController;
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
