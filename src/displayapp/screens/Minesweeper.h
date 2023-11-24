#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Minesweeper : public Screen {
      public:
        Minesweeper();
        ~Minesweeper() override;
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        bool OnTouchEvent(TouchEvents event);
        bool OnTouchEvent(uint16_t x, uint16_t y);
      };
    }
  }
}