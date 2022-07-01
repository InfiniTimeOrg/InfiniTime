#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class PageIndicator {
      public:
        PageIndicator(int nCurrentScreen, int nScreens);
        void Create();

      private:
        int nCurrentScreen;
        int nScreens;

        lv_point_t pageIndicatorBasePoints[2];
        lv_point_t pageIndicatorPoints[2];
        lv_obj_t* pageIndicatorBase;
        lv_obj_t* pageIndicator;
      };
    }
  }
}
