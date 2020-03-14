#pragma once

#include <libs/lvgl/src/lv_draw/lv_img_decoder.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class BleIcon {
      public:
        static lv_img_dsc_t* GetIcon(bool isConnected);
      };
    }
  }
}