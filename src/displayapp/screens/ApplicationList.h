#pragma once

#include <array>
#include <memory>

#include "displayapp/ScreenIds.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/Tile.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ApplicationList : public Screen {
      public:
        explicit ApplicationList(DisplayApp* app,
                                 Pinetime::Controllers::Settings& settingsController,
                                 const Pinetime::Controllers::Battery& batteryController,
                                 const Pinetime::Controllers::Ble& bleController,
                                 Controllers::DateTime& dateTimeController);
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

        static constexpr int appsPerScreen = 6;

        // Increment this when more space is needed
        static constexpr int nScreens = 2;

        static constexpr std::array<Tile::Applications, appsPerScreen * nScreens> applications {{
          {Symbols::stopWatch, ScreenId::StopWatch},
          {Symbols::clock, ScreenId::Alarm},
          {Symbols::hourGlass, ScreenId::Timer},
          {Symbols::shoe, ScreenId::Steps},
          {Symbols::heartBeat, ScreenId::HeartRate},
          {Symbols::music, ScreenId::Music},

          {Symbols::paintbrush, ScreenId::Paint},
          {Symbols::paddle, ScreenId::Paddle},
          {"2", ScreenId::Twos},
          {Symbols::drum, ScreenId::Metronome},
          {Symbols::map, ScreenId::Navigation},
          {Symbols::none, ScreenId::None},

          // {"M", ScreenId::Motion},
        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
