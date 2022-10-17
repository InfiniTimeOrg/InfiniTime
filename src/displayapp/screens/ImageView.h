#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/DisplayApp.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ImageView : public Screen {
      public:
        static constexpr const char* directory = "/gallery";

        ImageView(DisplayApp* app, Pinetime::Controllers::FS& filesystem);
        ~ImageView() override;
      private:
        Pinetime::Controllers::FS& filesystem;
        void listdir();
        int nScreens;
      };
    }
  }
}
