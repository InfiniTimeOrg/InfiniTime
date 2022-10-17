#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ImageView.h"
#include "displayapp/DisplayApp.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Gallery : public Screen {
      public:
        static constexpr const char* directory = "/gallery/";

        Gallery(DisplayApp* app, Pinetime::Controllers::FS& filesystem);
        ~Gallery() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
      private:
        Pinetime::Controllers::FS& filesystem;
        std::unique_ptr<ImageView> current;

        void listdir();
        bool open(int n, DisplayApp::FullRefreshDirections direction);
        int nScreens;
        int index;
      };
    }
  }
}
