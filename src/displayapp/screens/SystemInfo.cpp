#include "SystemInfo.h"
#include <lvgl/lvgl.h>
#include "../DisplayApp.h"
#include "Label.h"
#include "Version.h"
#include "BootloaderVersion.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/brightness/BrightnessController.h"
#include "components/datetime/DateTimeController.h"
#include "components/motion/MotionController.h"
#include "drivers/Watchdog.h"

using namespace Pinetime::Applications::Screens;

namespace {
  const char* ToString(const Pinetime::Controllers::MotionController::DeviceTypes deviceType) {
    switch (deviceType) {
      case Pinetime::Controllers::MotionController::DeviceTypes::BMA421:
        return "BMA421";
      case Pinetime::Controllers::MotionController::DeviceTypes::BMA425:
        return "BMA425";
      case Pinetime::Controllers::MotionController::DeviceTypes::Unknown:
        return "???";
    }
    return "???";
  }
}

SystemInfo::SystemInfo(Pinetime::Applications::DisplayApp* app,
                       Pinetime::Controllers::DateTime& dateTimeController,
                       Pinetime::Controllers::Battery& batteryController,
                       Pinetime::Controllers::BrightnessController& brightnessController,
                       Pinetime::Controllers::Ble& bleController,
                       Pinetime::Drivers::WatchdogView& watchdog,
                       Pinetime::Controllers::MotionController& motionController,
                       Pinetime::Drivers::Cst816S& touchPanel)
  : Screen(app),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    brightnessController {brightnessController},
    bleController {bleController},
    watchdog {watchdog},
    motionController{motionController},
    touchPanel{touchPanel},
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return CreateScreen1();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen2();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen3();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen4();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen5();
              }},
             Screens::ScreenListModes::UpDown} {
}

SystemInfo::~SystemInfo() {
  lv_obj_clean(lv_scr_act());
}

bool SystemInfo::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen1() {
  
  auto newScreen = std::make_unique<Screens::Label>(0, 5, app);

  newScreen->addLineCenter("#FFFF00 InfiniTime#");
  newScreen->addLine();
  newScreen->addLineCenter("#444444 Version# %ld.%ld.%ld", Version::Major(), Version::Minor(), Version::Patch());
  newScreen->addLineCenter("#444444 Short Ref# %s", Version::GitCommitHash());
  newScreen->addLineCenter("#444444 Build date#");
  newScreen->addLineCenter("%s", __TIME__);
  newScreen->addLineCenter("%s", __DATE__);
  newScreen->addLine();
  newScreen->addLineCenter("#444444 Bootloader# %s", BootloaderVersion::VersionString());

  return std::move(newScreen);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen2() {
  uint8_t batteryPercent = batteryController.PercentRemaining();
  auto resetReason = [this]() {
    switch (watchdog.ResetReason()) {
      case Drivers::Watchdog::ResetReasons::Watchdog:
        return "wtdg";
      case Drivers::Watchdog::ResetReasons::HardReset:
        return "hardr";
      case Drivers::Watchdog::ResetReasons::NFC:
        return "nfc";
      case Drivers::Watchdog::ResetReasons::SoftReset:
        return "softr";
      case Drivers::Watchdog::ResetReasons::CpuLockup:
        return "cpulock";
      case Drivers::Watchdog::ResetReasons::SystemOff:
        return "off";
      case Drivers::Watchdog::ResetReasons::LpComp:
        return "lpcomp";
      case Drivers::Watchdog::ResetReasons::DebugInterface:
        return "dbg";
      case Drivers::Watchdog::ResetReasons::ResetPin:
        return "rst";
      default:
        return "?";
    }
  }();

  // uptime
  static constexpr uint32_t secondsInADay = 60 * 60 * 24;
  static constexpr uint32_t secondsInAnHour = 60 * 60;
  static constexpr uint32_t secondsInAMinute = 60;
  uint32_t uptimeSeconds = dateTimeController.Uptime().count();
  uint32_t uptimeDays = (uptimeSeconds / secondsInADay);
  uptimeSeconds = uptimeSeconds % secondsInADay;
  uint32_t uptimeHours = uptimeSeconds / secondsInAnHour;
  uptimeSeconds = uptimeSeconds % secondsInAnHour;
  uint32_t uptimeMinutes = uptimeSeconds / secondsInAMinute;
  uptimeSeconds = uptimeSeconds % secondsInAMinute;
  // TODO handle more than 100 days of uptime

  auto newScreen = std::make_unique<Screens::Label>(1, 5, app);
  newScreen->addLine("#444444 Date# %02hhu/%02hhu/%04hu",
                     dateTimeController.Day(),
                     static_cast<uint8_t>(dateTimeController.Month()),
                     dateTimeController.Year());
  newScreen->addLine(
    "#444444 Time# %02hhu:%02hhu:%02hhu", dateTimeController.Hours(), dateTimeController.Minutes(), dateTimeController.Seconds());
  newScreen->addLine("#444444 Uptime#");
  newScreen->addLine(" %02lud %02lu:%02lu:%02lu", uptimeDays, uptimeHours, uptimeMinutes, uptimeSeconds);
  newScreen->addLine("#444444 Battery# %hhu%%/%03humV", batteryPercent, batteryController.Voltage());
  newScreen->addLine("#444444 Backlight# %s", brightnessController.ToString());
  newScreen->addLine("#444444 Last reset# %s", resetReason);
  newScreen->addLine("#444444 Accel.# %s", ToString(motionController.DeviceType()));
  newScreen->addLine("#444444 Touch.# %hhx.%hhx.%hhx", touchPanel.GetChipId(), touchPanel.GetVendorId(), touchPanel.GetFwVersion());

  return std::move(newScreen);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen3() {
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);

  auto newScreen = std::make_unique<Screens::Label>(2, 5, app);
  auto& bleAddr = bleController.Address();

  newScreen->addLine("#444444 BLE MAC#");
  newScreen->addLine(" %02x:%02x:%02x:%02x:%02x:%02x", bleAddr[5], bleAddr[4], bleAddr[3], bleAddr[2], bleAddr[1], bleAddr[0]);
  newScreen->addLine("#444444 LVGL Memory#");
  newScreen->addLine(" #444444 used# %d (%d%%)", static_cast<int>(mon.total_size - mon.free_size), mon.used_pct);
  newScreen->addLine(" #444444 max used# %lu", mon.max_used);
  newScreen->addLine(" #444444 frag# %d%%", mon.frag_pct);
  newScreen->addLine(" #444444 free# %d", static_cast<int>(mon.free_biggest_size));
  newScreen->addLine("#444444 Steps# %i", 0);
  
  return std::move(newScreen);
}

bool SystemInfo::sortById(const TaskStatus_t& lhs, const TaskStatus_t& rhs) {
  return lhs.xTaskNumber < rhs.xTaskNumber;
}

std::unique_ptr<Screen> SystemInfo::CreateScreen4() {
  TaskStatus_t tasksStatus[10];
  lv_obj_t* infoTask = lv_table_create(lv_scr_act());
  lv_table_set_col_cnt(infoTask, 4);
  lv_table_set_row_cnt(infoTask, 8);
  lv_obj_set_pos(infoTask, 0, 10);

  lv_table_set_cell_value(infoTask, 0, 0, "#");
  lv_table_set_col_width(infoTask, 0, 30);
  lv_table_set_cell_value(infoTask, 0, 1, "S"); // State
  lv_table_set_col_width(infoTask, 1, 30);
  lv_table_set_cell_value(infoTask, 0, 2, "Task");
  lv_table_set_col_width(infoTask, 2, 80);
  lv_table_set_cell_value(infoTask, 0, 3, "Free");
  lv_table_set_col_width(infoTask, 3, 90);

  auto nb = uxTaskGetSystemState(tasksStatus, sizeof(tasksStatus) / sizeof(tasksStatus[0]), nullptr);
  std::sort(tasksStatus, tasksStatus + nb, sortById);
  for (uint8_t i = 0; i < nb && i < 7; i++) {

    lv_table_set_cell_value(infoTask, i + 1, 0, std::to_string(tasksStatus[i].xTaskNumber).c_str());
    char state[2] = {0};
    switch (tasksStatus[i].eCurrentState) {
      case eReady:
      case eRunning:
        state[0] = 'R';
        break;
      case eBlocked:
        state[0] = 'B';
        break;
      case eSuspended:
        state[0] = 'S';
        break;
      case eDeleted:
        state[0] = 'D';
        break;
      default:
        state[0] = 'I'; // Invalid
        break;
    }
    lv_table_set_cell_value(infoTask, i + 1, 1, state);
    lv_table_set_cell_value(infoTask, i + 1, 2, tasksStatus[i].pcTaskName);
    if (tasksStatus[i].usStackHighWaterMark < 20) {
      std::string str1 = std::to_string(tasksStatus[i].usStackHighWaterMark) + " low";
      lv_table_set_cell_value(infoTask, i + 1, 3, str1.c_str());
    } else {
      lv_table_set_cell_value(infoTask, i + 1, 3, std::to_string(tasksStatus[i].usStackHighWaterMark).c_str());
    }
  }
  return std::make_unique<Screens::Label>(3, 5, app, infoTask);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen5() {
  
  auto newScreen = std::make_unique<Screens::Label>(4, 5, app);
  
  newScreen->addLineCenter("Software Licensed");
  newScreen->addLineCenter("under the terms of");
  newScreen->addLineCenter("the GNU General");
  newScreen->addLineCenter("Public License v3");
  newScreen->addLine();
  newScreen->addLineCenter("#444444 Source code#");
  newScreen->addLineCenter("#FFFF00 git.io/inftime");
  
  return std::move(newScreen);
}
