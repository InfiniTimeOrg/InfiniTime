#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/PageIndicator.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Label : public Screen {
      public:
        Label(uint8_t screenID, uint8_t numScreens, lv_obj_t* labelText);
        ~Label() override;

      private:
        lv_obj_t* labelText = nullptr;
        Widgets::PageIndicator pageIndicator;
      };
    }
  }
}
