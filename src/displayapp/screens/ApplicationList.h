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
        static constexpr int nApps {5 + APP_METRONOME + APP_MUSIC + APP_NAVIGATION + APP_PADDLE + APP_PAINT + APP_TWOS + APP_MOTION};
        static constexpr int nScreens {(nApps / appsPerScreen) + ((nApps % appsPerScreen) != 0)};

        static constexpr std::array<Tile::Applications, appsPerScreen * nScreens> applications {{
          {Symbols::stopWatch, Apps::StopWatch},
          {Symbols::clock, Apps::Alarm},
          {Symbols::hourGlass, Apps::Timer},
          {Symbols::shoe, Apps::Steps},
          {Symbols::heartBeat, Apps::HeartRate},
          APP_MUSIC ? (Tile::Applications) {Symbols::music, Apps::Music} : (Tile::Applications) {nullptr, Apps::None},
          APP_PAINT ? (Tile::Applications) {Symbols::paintbrush, Apps::Paint} : (Tile::Applications) {nullptr, Apps::None},
          APP_PADDLE ? (Tile::Applications) {Symbols::paddle, Apps::Paddle} : (Tile::Applications) {nullptr, Apps::None},
          APP_TWOS ? (Tile::Applications) {"2", Apps::Twos} : (Tile::Applications) {nullptr, Apps::None},
          APP_MOTION ? (Tile::Applications) {Symbols::chartLine, Apps::Motion} : (Tile::Applications) {nullptr, Apps::None},
          APP_METRONOME ? (Tile::Applications) {Symbols::drum, Apps::Metronome} : (Tile::Applications) {nullptr, Apps::None},
          APP_NAVIGATION ? (Tile::Applications) {Symbols::map, Apps::Navigation} : (Tile::Applications) {nullptr, Apps::None},
        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
