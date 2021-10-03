#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "Screen.h"
#include "../Apps.h"
#include "components/settings/Settings.h"

#define MAXLISTITEMS 4

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class List : public Screen {
      public:
        struct Applications {
          const char* icon;
          const char* name;
          Pinetime::Applications::Apps application;
        };

        explicit List(uint8_t screenID,
                      uint8_t numScreens,
                      DisplayApp* app,
                      Controllers::Settings& settingsController,
                      std::array<Applications, MAXLISTITEMS>& applications);
        ~List() override;

        void OnButtonEvent(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
        Pinetime::Applications::Apps apps[MAXLISTITEMS];

        lv_obj_t* itemApps[MAXLISTITEMS];

        lv_point_t pageIndicatorBasePoints[2];
        lv_point_t pageIndicatorPoints[2];
        lv_obj_t* pageIndicatorBase;
        lv_obj_t* pageIndicator;
      };
    }
  }
}
