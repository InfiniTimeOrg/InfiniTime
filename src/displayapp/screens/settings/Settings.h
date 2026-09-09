#pragma once

#include <array>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/List.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class Settings : public Screen {
      public:
        Settings(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~Settings() override;

        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

      private:
        DisplayApp* app;
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        Controllers::Settings& settingsController;

        static constexpr int entriesPerScreen = 4;

        // Increment this when more space is needed
        static constexpr int nScreens = 4;

        static constexpr std::array<List::Applications, entriesPerScreen * nScreens> entries {{
          {Symbols::sun, Localization::StringId::Display, Apps::SettingDisplay},
          {Symbols::eye, Localization::StringId::WakeUp, Apps::SettingWakeUp},
          {Symbols::clock, Localization::StringId::TimeFormat, Apps::SettingTimeFormat},
          {Symbols::home, Localization::StringId::WatchFace, Apps::SettingWatchFace},

          {Symbols::shoe, Localization::StringId::Steps, Apps::SettingSteps},
          {Symbols::heartBeat, Localization::StringId::HeartRate, Apps::SettingHeartRate},
          {Symbols::clock, Localization::StringId::DateTime, Apps::SettingSetDateTime},
          {Symbols::cloudSunRain, Localization::StringId::Weather, Apps::SettingWeatherFormat},

          {Symbols::batteryHalf, Localization::StringId::Battery, Apps::BatteryInfo},
          {Symbols::clock, Localization::StringId::Chimes, Apps::SettingChimes},
          {Symbols::tachometer, Localization::StringId::ShakeCalibration, Apps::SettingShakeThreshold},
          {Symbols::check, Localization::StringId::Firmware, Apps::FirmwareValidation},

          {Symbols::shieldAlt, Localization::StringId::Ota, Apps::SettingOTA},
          {Symbols::bluetooth, Localization::StringId::Bluetooth, Apps::SettingBluetooth},
          {Symbols::map, Localization::StringId::Language, Apps::SettingLanguage},
          {Symbols::list, Localization::StringId::About, Apps::SysInfo},
        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
