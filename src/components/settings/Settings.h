#pragma once
#include <cstdint>
#include <bitset>
#include "components/datetime/DateTimeController.h"
#include "components/brightness/BrightnessController.h"
#include "components/fs/FS.h"
#include "drivers/Cst816s.h"

namespace Pinetime {
  namespace Controllers {
    class Settings {
    public:
      enum class ClockType : uint8_t { H24, H12 };
      enum class Vibration : uint8_t { ON, OFF };
      enum class WakeUpMode : uint8_t {
        SingleTap = 0,
        DoubleTap = 1,
        RaiseWrist = 2,
      };

      Settings(Pinetime::Controllers::FS& fs);

      void Init();
      void SaveSettings();

      void SetClockFace(uint8_t face) {
        if (face != settings.clockFace) {
          settingsChanged = true;
        }
        settings.clockFace = face;
      };
      uint8_t GetClockFace() const {
        return settings.clockFace;
      };

      void SetAppMenu(uint8_t menu) {
        appMenu = menu;
      };
      uint8_t GetAppMenu() {
        return appMenu;
      };

      void SetSettingsMenu(uint8_t menu) {
        settingsMenu = menu;
      };
      uint8_t GetSettingsMenu() const {
        return settingsMenu;
      };

      void SetClockType(ClockType clocktype) {
        if (clocktype != settings.clockType) {
          settingsChanged = true;
        }
        settings.clockType = clocktype;
      };
      ClockType GetClockType() const {
        return settings.clockType;
      };

      void SetVibrationStatus(Vibration status) {
        if (status != settings.vibrationStatus) {
          settingsChanged = true;
        }
        settings.vibrationStatus = status;
      };
      Vibration GetVibrationStatus() const {
        return settings.vibrationStatus;
      };

      void SetScreenTimeOut(uint32_t timeout) {
        if (timeout != settings.screenTimeOut) {
          settingsChanged = true;
        }
        settings.screenTimeOut = timeout;
      };
      uint32_t GetScreenTimeOut() const {
        return settings.screenTimeOut;
      };

      void setWakeUpMode(WakeUpMode wakeUp, bool enabled) {
        if (enabled != isWakeUpModeOn(wakeUp)) {
          settingsChanged = true;
        }
        settings.wakeUpMode.set(static_cast<size_t>(wakeUp), enabled);
        // Handle special behavior
        if (enabled) {
          switch (wakeUp) {
            case WakeUpMode::SingleTap:
              settings.wakeUpMode.set(static_cast<size_t>(WakeUpMode::DoubleTap), false);
              break;
            case WakeUpMode::DoubleTap:
              settings.wakeUpMode.set(static_cast<size_t>(WakeUpMode::SingleTap), false);
              break;
            case WakeUpMode::RaiseWrist:
              break;
          }
        }
      };

      std::bitset<3> getWakeUpModes() const {
        return settings.wakeUpMode;
      }

      bool isWakeUpModeOn(const WakeUpMode mode) const {
        return getWakeUpModes()[static_cast<size_t>(mode)];
      }

      void SetBrightness(Controllers::BrightnessController::Levels level) {
        if (level != settings.brightLevel) {
          settingsChanged = true;
        }
        settings.brightLevel = level;
      };
      Controllers::BrightnessController::Levels GetBrightness() const {
        return settings.brightLevel;
      };

      void SetStepsGoal( uint32_t goal ) { 
        if ( goal != settings.stepsGoal ) {
          settingsChanged = true;
        }
        settings.stepsGoal = goal; 
      };
      
      uint32_t GetStepsGoal() const { return settings.stepsGoal; };

    private:
      Pinetime::Controllers::FS& fs;

      static constexpr uint32_t settingsVersion = 0x0001;
      struct SettingsData {

        uint32_t version = settingsVersion;
        uint32_t stepsGoal = 10000;
        uint32_t screenTimeOut = 15000;

        ClockType clockType = ClockType::H24;
        Vibration vibrationStatus = Vibration::ON;

        uint8_t clockFace = 0;

        std::bitset<3> wakeUpMode {0};

        Controllers::BrightnessController::Levels brightLevel = Controllers::BrightnessController::Levels::Medium;
      };

      SettingsData settings;
      bool settingsChanged = false;

      uint8_t appMenu = 0;
      uint8_t settingsMenu = 0;

      void LoadSettingsFromFile();
      void SaveSettingsToFile();
    };
  }
}
