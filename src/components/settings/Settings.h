#pragma once
#include <cstdint>
#include <cstring>
#include "components/datetime/DateTimeController.h"
#include "components/brightness/BrightnessController.h"
#include "drivers/SpiNorFlash.h"
#include "drivers/Cst816s.h"
#include "lv_i18n/lv_i18n.h"

namespace Pinetime {
  namespace Controllers {
    class Settings {
    public:
      enum class ClockType { H24, H12 };
      enum class Vibration { ON, OFF };
      enum class WakeUpMode { None, SingleTap, DoubleTap, RaiseWrist };

      Settings(Pinetime::Drivers::SpiNorFlash& spiNorFlash);

      void Init();
      void SaveSettings();

      void SetClockFace(uint8_t face) {
        if (face != settings.clockFace)
          settingsChanged = true;
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
        if (clocktype != settings.clockType)
          settingsChanged = true;
        settings.clockType = clocktype;
      };
      ClockType GetClockType() const {
        return settings.clockType;
      };

      void SetVibrationStatus(Vibration status) {
        if (status != settings.vibrationStatus)
          settingsChanged = true;
        settings.vibrationStatus = status;
      };
      Vibration GetVibrationStatus() const {
        return settings.vibrationStatus;
      };

      void SetScreenTimeOut(uint32_t timeout) {
        if (timeout != settings.screenTimeOut)
          settingsChanged = true;
        settings.screenTimeOut = timeout;
      };
      uint32_t GetScreenTimeOut() const {
        return settings.screenTimeOut;
      };

      void setWakeUpMode(WakeUpMode wakeUp) {
        if (wakeUp != settings.wakeUpMode)
          settingsChanged = true;
        settings.wakeUpMode = wakeUp;
      };
      WakeUpMode getWakeUpMode() const {
        return settings.wakeUpMode;
      };

      void SetBrightness(Controllers::BrightnessController::Levels level) {
        if (level != settings.brightLevel)
          settingsChanged = true;
        settings.brightLevel = level;
      };
      Controllers::BrightnessController::Levels GetBrightness() const {
        return settings.brightLevel;
      };

      void SetStepsGoal( uint32_t goal ) { 
        if ( goal != settings.stepsGoal ) 
          settingsChanged = true;
        settings.stepsGoal = goal; 
      };
      
      uint32_t GetStepsGoal() const { return settings.stepsGoal; };

      void SetLocale(const char locale[]) {
        if (locale != settings.locale) {
	  settingsChanged = true;
	  lv_i18n_set_locale(locale);
	}
	strncpy(settings.locale, locale, 9);
      }
      const char* GetLocale() const {
        return settings.locale;
      }

    private:
      Pinetime::Drivers::SpiNorFlash& spiNorFlash;
      struct SettingsData {

        ClockType clockType = ClockType::H24;
        Vibration vibrationStatus = Vibration::ON;

        uint8_t clockFace = 0;

        uint32_t stepsGoal = 10000;
        uint32_t screenTimeOut = 15000;

        WakeUpMode wakeUpMode = WakeUpMode::None;

        Controllers::BrightnessController::Levels brightLevel = Controllers::BrightnessController::Levels::Medium;

	char locale[9];
      };

      SettingsData settings;
      bool settingsChanged = false;

      uint8_t appMenu = 0;
      uint8_t settingsMenu = 0;

      // There are 10 blocks of reserved flash to save settings
      // to minimize wear, the recording is done in a rotating way by the 10 blocks
      uint8_t settingsFlashBlock = 99; // default to indicate it needs to find the active block

      static constexpr uint32_t settingsBaseAddr = 0x3F6000; // Flash Settings Location
      static constexpr uint16_t settingsVersion = 0x0200;    // Flash Settings Version

      bool FindHeader();
      void ReadSettingsData();
      void EraseBlock();
      void SetHeader(bool state);
      void SaveSettingsData();
      void LoadSettingsFromFlash();
      void SaveSettingsToFlash();
    };
  }
}
