#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "displayapp/widgets/TwoDigitCounter.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class MTGLifePoints : public Screen {
      
      public:
        MTGLifePoints(DisplayApp* app);
        ~MTGLifePoints() override;

      private:
        Widgets::TwoDigitCounter twoDigitCounter = Widgets::TwoDigitCounter(0, 59, jetbrains_mono_76);
      };
      
    }
  }
}
