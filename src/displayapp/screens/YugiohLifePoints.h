#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/FourDigitCounter.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class YugiohLifePoints : public Screen {
        public:
          YugiohLifePoints(DisplayApp* app);
          ~YugiohLifePoints() override;
      };
    }
  }
}