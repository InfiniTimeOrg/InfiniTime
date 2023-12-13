#pragma once

#include <array>
#include <memory>
#include "displayapp/Apps.h"
#include "Screen.h"
#include "ScreenList.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include "Tile.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ApplicationList : public Screen {
      public:
        explicit ApplicationList(DisplayApp* app,
                                 Pinetime::Controllers::Settings& settingsController,
                                 const Pinetime::Controllers::Battery& batteryController,
                                 const Pinetime::Controllers::Ble& bleController,
                                 Controllers::DateTime& dateTimeController,
                                 Pinetime::Controllers::FS& filesystem,
                                 std::array<Tile::Applications, UserAppTypes::Count>&& apps);
        ~ApplicationList() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        DisplayApp* app;
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        Controllers::Settings& settingsController;
        const Pinetime::Controllers::Battery& batteryController;
        const Pinetime::Controllers::Ble& bleController;
        Controllers::DateTime& dateTimeController;
        Pinetime::Controllers::FS& filesystem;
        std::array<Tile::Applications, UserAppTypes::Count> apps;

        static constexpr int appsPerScreen = 6;

        static constexpr int nScreens = UserAppTypes::Count > 0 ? (UserAppTypes::Count - 1) / appsPerScreen + 1 : 1;

        ScreenList<nScreens> screens;
      };
    }
  }
}
