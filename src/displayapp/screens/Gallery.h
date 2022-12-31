#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/FileView.h"
#include "displayapp/DisplayApp.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Gallery : public Screen {
      public:
        static constexpr const char* index_file = "/gallery/index";

        Gallery(DisplayApp* app, Pinetime::Controllers::FS& filesystem);
        ~Gallery() override;
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

      private:
        int StringEndsWith(const char* str, const char* suffix);

        Pinetime::Controllers::FS& filesystem;
        std::unique_ptr<FileView> current;

        uint8_t CountFiles();
        bool Open(int n, DisplayApp::FullRefreshDirections direction);
        int nScreens;
        int index;
      };
    }
  }
}
