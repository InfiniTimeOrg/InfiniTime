#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Morse : public Screen {
      public:
        Morse(DisplayApp* app);
        ~Morse() override;
      };
    }
  }
}