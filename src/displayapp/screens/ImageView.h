#pragma once

#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ImageView : public Screen {
      public:
        ImageView(DisplayApp* app);
        ~ImageView() override;
      };
    }
  }
}
