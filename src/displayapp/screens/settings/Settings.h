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
        static constexpr int nScreens = 3;

        static constexpr std::array<List::Applications, entriesPerScreen * nScreens> entries {{
          {Symbols::sun, "Display", Apps::SettingDisplay},
          {Symbols::eye, "Wake Up", Apps::SettingWakeUp},
          {Symbols::clock, "Time format", Apps::SettingTimeFormat},
          {Symbols::home, "Watch face", Apps::SettingWatchFace},

          {Symbols::shoe, "Steps", Apps::SettingSteps},
          {Symbols::clock, "Date&Time", Apps::SettingSetDateTime},
          {Symbols::batteryHalf, "Battery", Apps::BatteryInfo},
          {Symbols::clock, "Chimes", Apps::SettingChimes},

          {Symbols::tachometer, "Shake Calib.", Apps::SettingShakeThreshold},
          {Symbols::check, "Firmware", Apps::FirmwareValidation},
          {Symbols::bluetooth, "Bluetooth", Apps::SettingBluetooth},
          {Symbols::list, "About", Apps::SysInfo},

          // {Symbols::none, "None", Apps::None},
          // {Symbols::none, "None", Apps::None},
          // {Symbols::none, "None", Apps::None},
          // {Symbols::none, "None", Apps::None},

        }};
        ScreenList<nScreens> screens;
      };
    }
  }
}
