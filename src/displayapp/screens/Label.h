#pragma once

#include "Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Label : public Screen {
        public:
          Label(DisplayApp* app, const char* text);
          ~Label() override;
          bool Refresh() override {return false;}

        private:
          lv_obj_t * label = nullptr;
          const char* text = nullptr;
      };
    }
  }
}