#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "displayapp/screens/ScreenList.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/screens/WatchFaceInfineat.h"
#include "displayapp/screens/WatchFaceCasioStyleG7710.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingWatchFace : public Screen {
      public:
        struct Item {
          const char* name;
          WatchFace watchface;
          bool enabled;
        };

        SettingWatchFace(DisplayApp* app,
                         std::array<Item, UserWatchFaceTypes::Count>&& watchfaceItems,
                         Pinetime::Controllers::Settings& settingsController,
                         Pinetime::Controllers::FS& filesystem);
        ~SettingWatchFace() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        DisplayApp* app;
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        static constexpr int settingsPerScreen = 4;
        std::array<Item, UserWatchFaceTypes::Count> watchfaceItems;
        static constexpr int nScreens = UserWatchFaceTypes::Count > 0 ? (UserWatchFaceTypes ::Count - 1) / settingsPerScreen + 1 : 1;

        Controllers::Settings& settingsController;
        Pinetime::Controllers::FS& filesystem;

        static constexpr const char* title = "Watch face";
        static constexpr const char* symbol = Symbols::home;

        ScreenList<nScreens> screens;
      };
    }
  }
}
