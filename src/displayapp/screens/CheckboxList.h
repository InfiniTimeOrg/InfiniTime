#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/Apps.h"
#include "components/settings/Settings.h"

#define MAXLISTITEMS 4

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class CheckboxList : public Screen {
      public:
        CheckboxList(const uint8_t screenID,
                     const uint8_t numScreens,
                     DisplayApp* app,
                     Controllers::Settings& settingsController,
                     const char* optionsTitle,
                     const char* optionsSymbol,
                     void (Controllers::Settings::*SetOptionIndex)(uint8_t),
                     uint8_t (Controllers::Settings::*GetOptionIndex)() const,
                     std::array<const char*, MAXLISTITEMS> options);

        ~CheckboxList() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        const uint8_t screenID;
        Controllers::Settings& settingsController;
        const char* optionsTitle;
        const char* optionsSymbol;
        void (Controllers::Settings::*SetOptionIndex)(uint8_t);
        uint8_t (Controllers::Settings::*GetOptionIndex)() const;
        std::array<const char*, MAXLISTITEMS> options;

        lv_obj_t* cbOption[MAXLISTITEMS];

        lv_point_t pageIndicatorBasePoints[2];
        lv_point_t pageIndicatorPoints[2];
        lv_obj_t* pageIndicatorBase;
        lv_obj_t* pageIndicator;
      };
    }
  }
}
