#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/PageIndicator.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class Page : public Screen {
      public:
        Page(uint8_t screenID, uint8_t numScreens, lv_obj_t* contentObj) : contentObj {contentObj}, pageIndicator(screenID, numScreens) {
          pageIndicator.Create();
        }

        ~Page() override {
          lv_obj_clean(lv_scr_act());
        }

      private:
        lv_obj_t* contentObj = nullptr;
        Widgets::PageIndicator pageIndicator;
      };
    }
  }
}
