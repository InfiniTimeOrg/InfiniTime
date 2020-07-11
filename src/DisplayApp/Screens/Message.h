#pragma once

#include <cstdint>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include <lvgl/src/lv_core/lv_style.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Message : public Screen{
        public:
          explicit Message(DisplayApp* app);
          ~Message() override;
          bool Refresh() override;
          bool OnButtonPushed();
          void OnObjectEvent(lv_obj_t* obj, lv_event_t event);

        private:
          lv_obj_t * label;
          lv_obj_t* backgroundLabel;
          lv_obj_t * button;
          lv_obj_t * labelClick;

          uint32_t clickCount = 0 ;
          uint32_t previousClickCount = 0;
          bool running = true;
      };
    }
  }
}
