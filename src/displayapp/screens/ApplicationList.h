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
          {Symbols::stopWatch, Apps::StopWatch},
          {Symbols::clock, Apps::Alarm},
          {Symbols::hourGlass, Apps::Timer},
          {Symbols::shoe, Apps::Steps},
          {Symbols::heartBeat, Apps::HeartRate},
          {isDisabled(Apps::Music) ? Symbols::none : Symbols::music, isDisabled(Apps::Music) ? Apps::None : Apps::Music},

          {isDisabled(Apps::Paint) ? Symbols::none : Symbols::paintbrush, isDisabled(Apps::Paint) ? Apps::None : Apps::Paint},
          {isDisabled(Apps::Paddle) ? Symbols::none : Symbols::paddle, isDisabled(Apps::Paddle) ? Apps::None : Apps::Paddle},
          {isDisabled(Apps::Twos) ? Symbols::none : "2", isDisabled(Apps::Twos) ? Apps::None : Apps::Twos},
          {isDisabled(Apps::Metronome) ? Symbols::none : Symbols::drum, isDisabled(Apps::Metronome) ? Apps::None : Apps::Metronome},
          {isDisabled(Apps::Navigation) ? Symbols::none : Symbols::map, isDisabled(Apps::Navigation) ? Apps::None : Apps::Navigation},
          {isDisabled(Apps::Motion) ? Symbols::none : "M", isDisabled(Apps::Motion) ? Apps::None : Apps::Motion},
        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
