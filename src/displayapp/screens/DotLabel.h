#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/DotIndicator.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class DotLabel : public Screen {
      public:
        DotLabel(uint8_t screenID, uint8_t numScreens, DisplayApp* app, lv_obj_t* labelText);
        ~DotLabel() override;

      private:
        lv_obj_t* labelText = nullptr;
        Widgets::DotIndicator dotIndicator;
      };
    }
  }
}
