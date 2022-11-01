#pragma once

#include <array>
#include <memory>

#include "displayapp/AvailableApps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/Tile.h"

#define N_APPS (5 + APP_METRONOME + APP_MUSIC + APP_NAVIGATION + APP_PADDLE + APP_PAINT + APP_TWOS + APP_MOTION)

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ApplicationList : public Screen {
      public:
        explicit ApplicationList(DisplayApp* app,
                                 Pinetime::Controllers::Settings& settingsController,
                                 Pinetime::Controllers::Battery& batteryController,
                                 Pinetime::Controllers::Ble& bleController,
                                 Controllers::DateTime& dateTimeController);
        ~ApplicationList() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        Controllers::Settings& settingsController;
        Pinetime::Controllers::Battery& batteryController;
        Pinetime::Controllers::Ble& bleController;
        Controllers::DateTime& dateTimeController;

        static constexpr int appsPerScreen {6};
        static constexpr int nScreens {(N_APPS / 6) + ((N_APPS % 6) != 0)};

        static constexpr std::array<Tile::Applications, appsPerScreen * nScreens> applications {{
          {Symbols::stopWatch, Apps::StopWatch},
          {Symbols::clock, Apps::Alarm},
          {Symbols::hourGlass, Apps::Timer},
          {Symbols::shoe, Apps::Steps},
          {Symbols::heartBeat, Apps::HeartRate},
#if APP_MUSIC
          {Symbols::music, Apps::Music},
#endif
#if APP_PAINT
          {Symbols::paintbrush, Apps::Paint},
#endif
#if APP_PADDLE
          {Symbols::paddle, Apps::Paddle},
#endif
#if APP_TWOS
          {"2", Apps::Twos},
#endif
#if APP_MOTION
          {Symbols::chartLine, Apps::Motion},
#endif
#if APP_METRONOME
          {Symbols::drum, Apps::Metronome},
#endif
#if APP_NAVIGATION
          {Symbols::map, Apps::Navigation},
#endif
        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
