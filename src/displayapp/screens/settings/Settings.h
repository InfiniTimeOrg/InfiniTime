#pragma once

#include <array>
#include <memory>
#include "displayapp/ScreenIds.h"
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
        static constexpr int nScreens = 3;

        static constexpr std::array<List::Applications, entriesPerScreen * nScreens> entries {{
          {Symbols::sun, "Display", ScreenId::SettingDisplay},
          {Symbols::eye, "Wake Up", ScreenId::SettingWakeUp},
          {Symbols::clock, "Time format", ScreenId::SettingTimeFormat},
          {Symbols::home, "Watch face", ScreenId::SettingWatchFace},

          {Symbols::shoe, "Steps", ScreenId::SettingSteps},
          {Symbols::clock, "Date&Time", ScreenId::SettingSetDateTime},
          {Symbols::batteryHalf, "Battery", ScreenId::BatteryInfo},
          {Symbols::clock, "Chimes", ScreenId::SettingChimes},

          {Symbols::tachometer, "Shake Calib.", ScreenId::SettingShakeThreshold},
          {Symbols::check, "Firmware", ScreenId::FirmwareValidation},
          {Symbols::bluetooth, "Bluetooth", ScreenId::SettingBluetooth},
          {Symbols::list, "About", ScreenId::SysInfo},

          // {Symbols::none, "None", ScreenId::None},
          // {Symbols::none, "None", ScreenId::None},
          // {Symbols::none, "None", ScreenId::None},
          // {Symbols::none, "None", ScreenId::None},

        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
