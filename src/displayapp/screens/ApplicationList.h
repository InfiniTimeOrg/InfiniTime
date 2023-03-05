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
          {disabledApps & static_cast<uint64_t>(Apps::Music) ? Symbols::none : Symbols::music,
           disabledApps& static_cast<uint64_t>(Apps::Music) ? Apps::None : Apps::Music},

          {disabledApps & static_cast<uint64_t>(Apps::Paint) ? Symbols::none : Symbols::paintbrush,
           disabledApps& static_cast<uint64_t>(Apps::Paint) ? Apps::None : Apps::Paint},
          {disabledApps & static_cast<uint64_t>(Apps::Paddle) ? Symbols::none : Symbols::paddle,
           disabledApps& static_cast<uint64_t>(Apps::Paddle) ? Apps::None : Apps::Paddle},
          {disabledApps & static_cast<uint64_t>(Apps::Twos) ? Symbols::none : "2",
           disabledApps& static_cast<uint64_t>(Apps::Twos) ? Apps::None : Apps::Twos},
          {disabledApps & static_cast<uint64_t>(Apps::Metronome) ? Symbols::none : Symbols::drum,
           disabledApps& static_cast<uint64_t>(Apps::Metronome) ? Apps::None : Apps::Metronome},
          {disabledApps & static_cast<uint64_t>(Apps::Navigation) ? Symbols::none : Symbols::map,
           disabledApps& static_cast<uint64_t>(Apps::Navigation) ? Apps::None : Apps::Navigation},
          {disabledApps & static_cast<uint64_t>(Apps::Motion) ? Symbols::none : "M",
           disabledApps& static_cast<uint64_t>(Apps::Motion) ? Apps::None : Apps::Motion},
        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
