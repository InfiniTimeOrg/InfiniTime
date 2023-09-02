#pragma once

#include <array>
#include <memory>

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/Tile.h"
#include "displayapp/screens/Navigation.h"

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
                                 Pinetime::Controllers::FS& filesystem);
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

        static constexpr int appsPerScreen = 6;

        // Increment this when more space is needed
        static constexpr int nScreens = 2;

        std::array<Tile::Applications, appsPerScreen * nScreens> applications {{
          {Symbols::stopWatch, Apps::StopWatch, true},
          {Symbols::clock, Apps::Alarm, true},
          {Symbols::hourGlass, Apps::Timer, true},
          {Symbols::shoe, Apps::Steps, true},
          {Symbols::heartBeat, Apps::HeartRate, true},
          {Symbols::music, Apps::Music, true},

          {Symbols::paintbrush, Apps::Paint, true},
          {Symbols::paddle, Apps::Paddle, true},
          {"2", Apps::Twos, true},
          {Symbols::drum, Apps::Metronome, true},
          {Symbols::map, Apps::Navigation, Applications::Screens::Navigation::IsAvailable(filesystem)},
          {Symbols::none, Apps::None, false},

          // {"M", Apps::Motion},
        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
