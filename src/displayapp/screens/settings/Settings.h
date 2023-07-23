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
          {Symbols::sun, "Display", static_cast<uint8_t>(Apps::SettingDisplay)},
          {Symbols::eye, "Wake Up", static_cast<uint8_t>(Apps::SettingWakeUp)},
          {Symbols::clock, "Time format", static_cast<uint8_t>(Apps::SettingTimeFormat)},
          {Symbols::home, "Watch face", static_cast<uint8_t>(Apps::SettingWatchFace)},

          {Symbols::shoe, "Steps", static_cast<uint8_t>(Apps::SettingSteps)},
          {Symbols::clock, "Date&Time", static_cast<uint8_t>(Apps::SettingSetDateTime)},
          {Symbols::batteryHalf, "Battery", static_cast<uint8_t>(Apps::BatteryInfo)},
          {Symbols::clock, "Chimes", static_cast<uint8_t>(Apps::SettingChimes)},

          {Symbols::tachometer, "Shake Calib.", static_cast<uint8_t>(Apps::SettingShakeThreshold)},
          {Symbols::check, "Firmware", static_cast<uint8_t>(Apps::FirmwareValidation)},
          {Symbols::bluetooth, "Bluetooth", static_cast<uint8_t>(Apps::SettingBluetooth)},
          {Symbols::list, "About", static_cast<uint8_t>(Apps::SysInfo)},

          // {Symbols::none, "None", Apps::None},
          // {Symbols::none, "None", Apps::None},
          // {Symbols::none, "None", Apps::None},
          // {Symbols::none, "None", Apps::None},

        }};
        ScreenList screens;
      };
    }
  }
}
