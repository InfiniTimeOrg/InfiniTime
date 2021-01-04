#pragma once

#include <cstdint>
#include "Screen.h"
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class DropDownDemo : public Screen {
        public:
          DropDownDemo(DisplayApp* app);
          ~DropDownDemo() override;

          bool Refresh() override;
          bool OnButtonPushed() override;
          bool OnTouchEvent(TouchEvents event) override;

        private:
          lv_obj_t * ddlist;
          bool running = true;
          bool isDropDownOpened = false;
      };
    }
  }
}
