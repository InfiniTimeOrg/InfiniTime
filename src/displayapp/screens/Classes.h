#pragma once

#include <cstdint>
#include <chrono>
#include "displayapp/screens/Screen.h"
#include <lvgl/src/lv_core/lv_style.h>
#include <lvgl/src/lv_core/lv_obj.h>
//#include <components/motion/MotionController.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Classes : public Screen {
      public:
        //Classes(Controllers::DateTime& dateTimeController);
        Classes();
        ~Classes() override;

        void Refresh() override;

      private:
      
        lv_obj_t* label;

      };
    }
  }
}
