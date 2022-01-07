#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class CoinFlip : public Screen {
      public:
        CoinFlip(DisplayApp* app);
        ~CoinFlip() override;
      };
    }
  }
}
