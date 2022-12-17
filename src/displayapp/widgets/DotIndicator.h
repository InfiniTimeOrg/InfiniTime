#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class DotIndicator {
      public:
        DotIndicator(uint8_t nCurrentScreen, uint8_t nScreens);
        void Create();

      private:
        uint8_t nCurrentScreen;
        uint8_t nScreens;

        lv_obj_t* dotIndicator[4];
      };
    }
  }
}
