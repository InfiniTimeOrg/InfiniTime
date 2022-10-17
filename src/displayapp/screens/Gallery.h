#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/DisplayApp.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Gallery : public Screen {
      public:
        static constexpr const char* directory = "/gallery";

        Gallery(DisplayApp* app, Pinetime::Controllers::FS& filesystem);
        ~Gallery() override;
      private:
        Pinetime::Controllers::FS& filesystem;

        void listdir();
        void open(int n);
        int nScreens;
      };
    }
  }
}
