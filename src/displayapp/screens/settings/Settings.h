#pragma once

#include <cstdint>
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
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        Controllers::Settings& settingsController;

        static constexpr std::array<List::Applications, 16> entries {{
          {Symbols::sun, "Display", Apps::SettingDisplay},
          {Symbols::eye, "Wake Up", Apps::SettingWakeUp},
          {Symbols::clock, "Time format", Apps::SettingTimeFormat},
          {Symbols::home, "Watch face", Apps::SettingWatchFace},

          {Symbols::shoe, "Steps", Apps::SettingSteps},
          {Symbols::clock, "Set date", Apps::SettingSetDate},
          {Symbols::clock, "Set time", Apps::SettingSetTime},
          {Symbols::batteryHalf, "Battery", Apps::BatteryInfo},

          {Symbols::clock, "Chimes", Apps::SettingChimes},
          {Symbols::tachometer, "Shake Calib.", Apps::SettingShakeThreshold},
          {Symbols::check, "Firmware", Apps::FirmwareValidation},
          {Symbols::bluetooth, "Bluetooth", Apps::SettingBluetooth},

          {Symbols::list, "About", Apps::SysInfo},
          {Symbols::none, "None", Apps::None},
          {Symbols::none, "None", Apps::None},
          {Symbols::none, "None", Apps::None},
        }};
        static constexpr int nScreens = (entries.size() + 3) / 4;
        ScreenList<nScreens> screens;
      };
    }
  }
}
