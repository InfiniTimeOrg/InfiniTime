#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Container.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SettingWatchFace : public Screen {
      public:
        SettingWatchFace(DisplayApp* app, Controllers::Settings& settingsController);
        ~SettingWatchFace() override;

        bool OnTouchEvent(Applications::TouchEvents event) override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        static constexpr const char* icon = Symbols::home;
        static constexpr const char* title = "Watch face";

        static constexpr uint8_t optionsPerScreen = 4;
        static constexpr uint8_t nOptions = 6;
        static constexpr uint8_t nScreens = (nOptions + optionsPerScreen - 1) / optionsPerScreen;
        static constexpr std::array<const char*, nOptions> options = {
            "Digital face", "Analog face", "PineTimeStyle", "Terminal", 
            "Infineat", "Fuzzy face",
        };

        auto CreateScreenList();
        std::unique_ptr<Screen> CreateScreen(uint8_t screenIdx);

        Controllers::Settings& settingsController;

        ScreenList<nScreens> screens;

        lv_obj_t* checkboxes[optionsPerScreen];
      };
    }
  }
}
