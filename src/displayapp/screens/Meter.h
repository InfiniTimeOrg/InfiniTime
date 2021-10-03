#pragma once

#include <cstdint>
#include "Screen.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>

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

        uint32_t value = 0;

        lv_task_t* taskRefresh;
      };
    }
  }
}
