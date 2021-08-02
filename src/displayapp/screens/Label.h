#pragma once

#include "Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Label : public Screen {
      public:
        Label(uint8_t screenID, uint8_t numScreens, DisplayApp* app, lv_obj_t* labelText);
        ~Label() override;

        bool Refresh() override;

      private:
        lv_obj_t* labelText = nullptr;
        lv_point_t pageIndicatorBasePoints[2];
        lv_point_t pageIndicatorPoints[2];
        lv_obj_t* pageIndicatorBase;
        lv_obj_t* pageIndicator;
      };
    }
  }
}
