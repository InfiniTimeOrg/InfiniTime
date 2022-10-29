#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class MTGLifePoints : public Screen {
      public:
        MTGLifePoints(DisplayApp* app);
        ~MTGLifePoints() override;
      };
    }
  }
}