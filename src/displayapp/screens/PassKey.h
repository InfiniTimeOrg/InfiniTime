#pragma once

#include "Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class PassKey : public Screen {
      public:
        PassKey(DisplayApp* app, uint32_t key);
        ~PassKey() override;

      private:
        lv_obj_t* lpasskey;
      };
    }
  }
}
