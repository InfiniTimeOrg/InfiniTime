#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/DisplayApp.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ImageView : public Screen {
      public:
        ImageView(DisplayApp* app, const char *path);
        ~ImageView() override;
      private:
        char name[LFS_NAME_MAX];
      };
    }
  }
}

