#include <FreeRTOS.h>
#include <algorithm>
#include <task.h>
#include "displayapp/screens/SystemInfo.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Label.h"
#include "Version.h"
#include "BootloaderVersion.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/brightness/BrightnessController.h"
#include "components/datetime/DateTimeController.h"
#include "components/motion/MotionController.h"
#include "drivers/Watchdog.h"
#include "displayapp/InfiniTimeTheme.h"

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
                       const Pinetime::Controllers::Battery& batteryController,
                       Pinetime::Controllers::BrightnessController& brightnessController,
                       const Pinetime::Controllers::Ble& bleController,
                       const Pinetime::Drivers::Watchdog& watchdog,
                       Pinetime::Controllers::MotionController& motionController,
                       const Pinetime::Drivers::Cst816S& touchPanel)
  : app {app},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    brightnessController {brightnessController},
    bleController {bleController},
    watchdog {watchdog},
    motionController {motionController},
    touchPanel {touchPanel},
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
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_fmt(label,
                        "#FFFF00 InfiniTime#\n\n"
                        "#808080 Version# %ld.%ld.%ld\n"
                        "#808080 Short Ref# %s\n"
                        "#808080 Build date#\n"
                        "%s\n"
                        "%s\n\n"
                        "#808080 Bootloader# %s",
                        Version::Major(),
                        Version::Minor(),
                        Version::Patch(),
                        Version::GitCommitHash(),
                        __DATE__,
                        __TIME__,
                        BootloaderVersion::VersionString());
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::make_unique<Screens::Label>(0, 5, label);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen2() {
  auto batteryPercent = batteryController.PercentRemaining();
  const auto* resetReason = [this]() {
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

#ifndef TARGET_DEVICE_NAME
  #define TARGET_DEVICE_NAME "UNKNOWN"
#endif

  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_fmt(label,
                        "#808080 Date# %04d-%02d-%02d\n"
                        "#808080 Time# %02d:%02d:%02d\n"
                        "#808080 Uptime#\n %02lud %02lu:%02lu:%02lu\n"
                        "#808080 Battery# %d%%/%03imV\n"
                        "#808080 Backlight# %s\n"
                        "#808080 Last reset# %s\n"
                        "#808080 Accel.# %s\n"
                        "#808080 Touch.# %x.%x.%x\n"
                        "#808080 Model# %s",
                        dateTimeController.Year(),
                        static_cast<uint8_t>(dateTimeController.Month()),
                        dateTimeController.Day(),
                        dateTimeController.Hours(),
                        dateTimeController.Minutes(),
                        dateTimeController.Seconds(),
                        uptimeDays,
                        uptimeHours,
                        uptimeMinutes,
                        uptimeSeconds,
                        batteryPercent,
                        batteryController.Voltage(),
                        brightnessController.ToString(),
                        resetReason,
                        ToString(motionController.DeviceType()),
                        touchPanel.GetChipId(),
                        touchPanel.GetVendorId(),
                        touchPanel.GetFwVersion(),
                        TARGET_DEVICE_NAME);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::make_unique<Screens::Label>(1, 5, label);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen3() {
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);

  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  const auto& bleAddr = bleController.Address();
  lv_label_set_text_fmt(label,
                        "#808080 BLE MAC#\n"
                        " %02x:%02x:%02x:%02x:%02x:%02x"
                        "\n"
                        "#808080 LVGL Memory#\n"
                        " #808080 used# %d (%d%%)\n"
                        " #808080 max used# %lu\n"
                        " #808080 frag# %d%%\n"
                        " #808080 free# %d",
                        bleAddr[5],
                        bleAddr[4],
                        bleAddr[3],
                        bleAddr[2],
                        bleAddr[1],
                        bleAddr[0],
                        static_cast<int>(mon.total_size - mon.free_size),
                        mon.used_pct,
                        mon.max_used,
                        mon.frag_pct,
                        static_cast<int>(mon.free_biggest_size));
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::make_unique<Screens::Label>(2, 5, label);
}

bool SystemInfo::sortById(const TaskStatus_t& lhs, const TaskStatus_t& rhs) {
  return lhs.xTaskNumber < rhs.xTaskNumber;
}

std::unique_ptr<Screen> SystemInfo::CreateScreen4() {
  static constexpr uint8_t maxTaskCount = 9;
  TaskStatus_t tasksStatus[maxTaskCount];

  lv_obj_t* infoTask = lv_table_create(lv_scr_act(), nullptr);
  lv_table_set_col_cnt(infoTask, 4);
  lv_table_set_row_cnt(infoTask, maxTaskCount + 1);
  lv_obj_set_style_local_pad_all(infoTask, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_border_color(infoTask, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, Colors::lightGray);

  lv_table_set_cell_value(infoTask, 0, 0, "#");
  lv_table_set_col_width(infoTask, 0, 30);
  lv_table_set_cell_value(infoTask, 0, 1, "S"); // State
  lv_table_set_col_width(infoTask, 1, 30);
  lv_table_set_cell_value(infoTask, 0, 2, "Task");
  lv_table_set_col_width(infoTask, 2, 80);
  lv_table_set_cell_value(infoTask, 0, 3, "Free");
  lv_table_set_col_width(infoTask, 3, 90);

  auto nb = uxTaskGetSystemState(tasksStatus, maxTaskCount, nullptr);
  std::sort(tasksStatus, tasksStatus + nb, sortById);
  for (uint8_t i = 0; i < nb && i < maxTaskCount; i++) {
    char buffer[7] = {0};

    sprintf(buffer, "%lu", tasksStatus[i].xTaskNumber);
    lv_table_set_cell_value(infoTask, i + 1, 0, buffer);
    switch (tasksStatus[i].eCurrentState) {
      case eReady:
      case eRunning:
        buffer[0] = 'R';
        break;
      case eBlocked:
        buffer[0] = 'B';
        break;
      case eSuspended:
        buffer[0] = 'S';
        break;
      case eDeleted:
        buffer[0] = 'D';
        break;
      default:
        buffer[0] = 'I'; // Invalid
        break;
    }
    buffer[1] = '\0';
    lv_table_set_cell_value(infoTask, i + 1, 1, buffer);
    lv_table_set_cell_value(infoTask, i + 1, 2, tasksStatus[i].pcTaskName);
    if (tasksStatus[i].usStackHighWaterMark < 20) {
      sprintf(buffer, "%d low", tasksStatus[i].usStackHighWaterMark);
    } else {
      sprintf(buffer, "%d", tasksStatus[i].usStackHighWaterMark);
    }
    lv_table_set_cell_value(infoTask, i + 1, 3, buffer);
  }
  return std::make_unique<Screens::Label>(3, 5, infoTask);
}

std::unique_ptr<Screen> SystemInfo::CreateScreen5() {
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_static(label,
                           "Software Licensed\n"
                           "under the terms of\n"
                           "the GNU General\n"
                           "Public License v3\n"
                           "#808080 Source code#\n"
                           "#FFFF00 https://github.com/#\n"
                           "#FFFF00 InfiniTimeOrg/#\n"
                           "#FFFF00 InfiniTime#");
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::make_unique<Screens::Label>(4, 5, label);
}
