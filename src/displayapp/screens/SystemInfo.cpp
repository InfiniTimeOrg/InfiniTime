#include "SystemInfo.h"
#include <lvgl/lvgl.h>
#include "../DisplayApp.h"
#include "Label.h"
#include "Version.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/brightness/BrightnessController.h"
#include "components/datetime/DateTimeController.h"
#include "drivers/Watchdog.h"

using namespace Pinetime::Applications::Screens;

SystemInfo::SystemInfo(Pinetime::Applications::DisplayApp *app,
                       Pinetime::Controllers::DateTime &dateTimeController,
                       Pinetime::Controllers::Battery& batteryController,
                       Pinetime::Controllers::BrightnessController& brightnessController,
                       Pinetime::Controllers::Ble& bleController,
                       Pinetime::Drivers::WatchdogView& watchdog) :
        Screen(app),
        dateTimeController{dateTimeController}, batteryController{batteryController},
        brightnessController{brightnessController}, bleController{bleController}, watchdog{watchdog},
        screens{app, {
                [this]() -> std::unique_ptr<Screen> { return CreateScreen1(); },
                [this]() -> std::unique_ptr<Screen> { return CreateScreen2(); },
                [this]() -> std::unique_ptr<Screen> { return CreateScreen3(); }
          }
        } {}


SystemInfo::~SystemInfo() {
  lv_obj_clean(lv_scr_act());
}

bool SystemInfo::Refresh() {
  screens.Refresh();
  return running;
}

bool SystemInfo::OnButtonPushed() {
  running = false;
  return true;
}

bool SystemInfo::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen1() {
  auto batteryPercent = static_cast<uint8_t>(batteryController.PercentRemaining());

  uint8_t brightness = 0;
  switch(brightnessController.Level()) {
    case Controllers::BrightnessController::Levels::Off: brightness = 0; break;
    case Controllers::BrightnessController::Levels::Low: brightness = 1; break;
    case Controllers::BrightnessController::Levels::Medium: brightness = 2; break;
    case Controllers::BrightnessController::Levels::High: brightness = 3; break;
  }
  auto resetReason = [this]() {
    switch (watchdog.ResetReason()) {
      case Drivers::Watchdog::ResetReasons::Watchdog: return "wtdg";
      case Drivers::Watchdog::ResetReasons::HardReset: return "hardr";
      case Drivers::Watchdog::ResetReasons::NFC: return "nfc";
      case Drivers::Watchdog::ResetReasons::SoftReset: return "softr";
      case Drivers::Watchdog::ResetReasons::CpuLockup: return "cpulock";
      case Drivers::Watchdog::ResetReasons::SystemOff: return "off";
      case Drivers::Watchdog::ResetReasons::LpComp: return "lpcomp";
      case Drivers::Watchdog::ResetReasons::DebugInterface: return "dbg";
      case Drivers::Watchdog::ResetReasons::ResetPin: return "rst";
      default: return "?";
    }
  }();

  // uptime
  static constexpr uint32_t secondsInADay = 60*60*24;
  static constexpr uint32_t secondsInAnHour = 60*60;
  static constexpr uint32_t secondsInAMinute = 60;
  uint32_t uptimeSeconds = dateTimeController.Uptime().count();
  uint32_t uptimeDays = (uptimeSeconds / secondsInADay);
  uptimeSeconds = uptimeSeconds % secondsInADay;
  uint32_t uptimeHours = uptimeSeconds / secondsInAnHour;
  uptimeSeconds = uptimeSeconds % secondsInAnHour;
  uint32_t uptimeMinutes = uptimeSeconds / secondsInAMinute;
  uptimeSeconds = uptimeSeconds % secondsInAMinute;
  // TODO handle more than 100 days of uptime

  sprintf(t1, "InfiniTime\n"
              "Version:%ld.%ld.%ld\n"
              "Build: %s\n"
              "       %s\n"
              "Date: %02d/%02d/%04d\n"
              "Time: %02d:%02d:%02d\n"
              "Uptime: %02lud %02lu:%02lu:%02lu\n"
              "Battery: %d%%\n"
              "Backlight: %d/3\n"
              "Last reset: %s\n",
          Version::Major(), Version::Minor(), Version::Patch(),
          __DATE__, __TIME__,
          dateTimeController.Day(), static_cast<uint8_t>(dateTimeController.Month()), dateTimeController.Year(),
          dateTimeController.Hours(), dateTimeController.Minutes(), dateTimeController.Seconds(),
          uptimeDays, uptimeHours, uptimeMinutes, uptimeSeconds,
          batteryPercent, brightness, resetReason);

  return std::unique_ptr<Screen>(new Screens::Label(app, t1));
}

std::unique_ptr<Screen> SystemInfo::CreateScreen2() {
  auto& bleAddr = bleController.Address();
  sprintf(t2, "BLE MAC: \n  %02x:%02x:%02x:%02x:%02x:%02x",
          bleAddr[5], bleAddr[4], bleAddr[3], bleAddr[2], bleAddr[1], bleAddr[0]);
  return std::unique_ptr<Screen>(new Screens::Label(app, t2));
}

std::unique_ptr<Screen> SystemInfo::CreateScreen3() {
  sprintf(t3, "Hello from\nthe developer!\n"
              "Software Licensed\n"
              "under the terms of\n"
              "the GNU General\n"
              "Public License v3\n"
              "Source code:\n"
              "https://github.com/\n"
              "    JF002/Pinetime");
  return std::unique_ptr<Screen>(new Screens::Label(app, t3));
}
