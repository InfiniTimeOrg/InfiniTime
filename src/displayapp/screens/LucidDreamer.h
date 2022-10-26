#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class LucidDreamer : public Screen {
      public:
        LucidDreamer(DisplayApp* app);
        ~LucidDreamer() override;
      };
    }
  }
}
