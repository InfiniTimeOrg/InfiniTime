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
      template <size_t SIZE> constexpr std::array<Tile::Applications, SIZE> initApplications() {
        std::array<Tile::Applications, SIZE> temp_applications {{
          {Symbols::stopWatch, Apps::StopWatch},
          {Symbols::clock, Apps::Alarm},
          {Symbols::hourGlass, Apps::Timer},
          {Symbols::shoe, Apps::Steps},
          {Symbols::heartBeat, Apps::HeartRate},
        }};
      
        auto i = 5;
        if (APP_MUSIC) {
          temp_applications[i++] = {Symbols::music, Apps::Music};
        }
        if (APP_PAINT) {
          temp_applications[i++] = {Symbols::paintbrush, Apps::Paint};
        }
        if (APP_PADDLE) {
          temp_applications[i++] = {Symbols::paddle, Apps::Paddle};
        }
        if (APP_TWOS) {
          temp_applications[i++] = {"2", Apps::Twos};
        }
        if (APP_MOTION) {
          temp_applications[i++] = {Symbols::chartLine, Apps::Motion};
        }
        if (APP_METRONOME) {
          temp_applications[i++] = {Symbols::drum, Apps::Metronome};
        }
        if (APP_NAVIGATION) {
          temp_applications[i++] = {Symbols::map, Apps::Navigation};
        }
      
        return temp_applications;
      }

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


        static constexpr std::array<Tile::Applications, appsPerScreen * nScreens> applications = initApplications<appsPerScreen * nScreens>();
        ScreenList<nScreens> screens;
      };
    }
  }
}
