#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/widgets/PageIndicator.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class FileView : public Screen {
      public:
        FileView(uint8_t screenID, uint8_t nScreens, const char* path);
        ~FileView() override;

        void ShowInfo();
        void HideInfo();
        void ToggleInfo();

      private:
        char name[LFS_NAME_MAX];
        lv_obj_t* label;

        Widgets::PageIndicator pageIndicator;
        uint8_t screenID, nScreens;
      };

      class ImageView : public FileView {
      public:
        ImageView(uint8_t screenID, uint8_t nScreens, const char* path);
      };

      class TextView : public FileView {
      public:
        TextView(uint8_t screenID, uint8_t nScreens, const char* path, Pinetime::Controllers::FS& fs);
        ~TextView() override;

      private:
        char* buf;
      };
    }
  }
}
