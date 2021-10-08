#pragma once

#include <cstdint>
#include "Screen.h"
#include <lvgl/src/misc/lv_style.h>
#include <lvgl/src/core/lv_obj.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Meter : public Screen {
      public:
        Meter(DisplayApp* app);
        ~Meter() override;

        void Refresh() override;

      private:
        lv_style_t style_lmeter;
        lv_obj_t* lmeter;
        lv_meter_scale_t* lscale;
        lv_meter_indicator_t* lindicator;

        uint32_t value = 0;

        lv_timer_t* taskRefresh;
      };
    }
  }
}
