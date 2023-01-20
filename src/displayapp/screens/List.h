#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <array>
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/Apps.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class List : public Screen {
      public:
        static constexpr int maxListItems = 4;

        struct Applications {
          const char* icon;
          const char* name;
          Pinetime::Applications::Apps application;
        };

        explicit List(uint8_t screenID,
                      uint8_t numScreens,
                      DisplayApp* app,
                      Controllers::Settings& settingsController,
                      std::array<Applications, maxListItems>& applications);
        ~List() override;

        void OnButtonEvent(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
        Pinetime::Applications::Apps apps[maxListItems];

        lv_obj_t* itemApps[maxListItems];

        Widgets::PageIndicator pageIndicator;
      };
    }
  }
}
