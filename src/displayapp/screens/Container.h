#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/PageIndicator.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Container : public Screen {
      public:
        Container(DisplayApp* app, lv_obj_t* container, uint8_t screenIdx, uint8_t nScreens, bool horizontal = false);
        ~Container() override;

      private:
        lv_obj_t* container;
        Widgets::PageIndicator pageIndicator;
      };
    }
  }
}
