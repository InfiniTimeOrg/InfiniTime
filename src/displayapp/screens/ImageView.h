#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/DisplayApp.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ImageView : public Screen {
      public:
        ImageView(uint8_t screenID, uint8_t nScreens, DisplayApp* app, const char *path);
        ~ImageView() override;

        void ShowInfo();
        void HideInfo();
        void ToggleInfo();
      private:
        char name[LFS_NAME_MAX];
        lv_obj_t *label;

        uint8_t screenID, nScreens;
      };
    }
  }
}

