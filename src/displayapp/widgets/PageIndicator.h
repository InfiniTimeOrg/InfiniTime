#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class PageIndicator {
      public:
        PageIndicator(uint8_t nCurrentScreen, uint8_t nScreens);
        void Create();

      private:
        uint8_t nCurrentScreen;
        uint8_t nScreens;

        lv_point_t pageIndicatorBasePoints[2];
        lv_point_t pageIndicatorPoints[2];
        lv_obj_t* pageIndicatorBase;
        lv_obj_t* pageIndicator;
      };
    }
  }
}
