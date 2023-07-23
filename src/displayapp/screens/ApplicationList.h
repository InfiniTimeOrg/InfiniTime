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
#include "displayapp/screens/AppController.h"

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
                                 Pinetime::Applications::AppController& appController);
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
        const Pinetime::Applications::AppController& appController;

        static constexpr int appsPerScreen = 6;

        // Increment this when more space is needed
        // static constexpr int nScreens = 2;

        // static constexpr std::array<Tile::Applications, appsPerScreen * nScreens> applications {{
        //   {Symbols::stopWatch, Apps::StopWatch},
        //   {Symbols::clock, Apps::Alarm},
        //   {Symbols::hourGlass, Apps::Timer},
        //   {Symbols::shoe, Apps::Steps},
        //   {Symbols::heartBeat, Apps::HeartRate},
        //   {Symbols::music, Apps::Music},

        //   {Symbols::paintbrush, Apps::Paint},
        //   {Symbols::paddle, Apps::Paddle},
        //   {AppController::GetSymbol(static_cast<uint8_t>(Apps::Dynamic)), Apps::Dynamic},
        //   {Symbols::drum, Apps::Metronome},
        //   {Symbols::map, Apps::Navigation},
        //   {Symbols::none, Apps::None},

        //   // {"M", Apps::Motion},
        // }};
        ScreenList screens;
      };
    }
  }
}
