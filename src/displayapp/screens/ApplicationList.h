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
                                 Pinetime::Controllers::Battery& batteryController,
                                 Controllers::DateTime& dateTimeController);
        ~ApplicationList() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        Controllers::Settings& settingsController;
        Pinetime::Controllers::Battery& batteryController;
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
          {Symbols::music, Apps::Music},

          {Symbols::paintbrush, Apps::Paint},
          {Symbols::paddle, Apps::Paddle},
          {"2", Apps::Twos},
          {Symbols::chartLine, Apps::Motion},
          {Symbols::drum, Apps::Metronome},
          {Symbols::map, Apps::Navigation},
        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
