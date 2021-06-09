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

SystemInfo::SystemInfo(Pinetime::Applications::DisplayApp* app,
                       Pinetime::Controllers::DateTime& dateTimeController,
                       Pinetime::Controllers::Battery& batteryController,
                       Pinetime::Controllers::BrightnessController& brightnessController,
                       Pinetime::Controllers::Ble& bleController,
                       Pinetime::Drivers::WatchdogView& watchdog)
  : Screen(app),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    brightnessController {brightnessController},
    bleController {bleController},
    watchdog {watchdog},
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

bool SystemInfo::Refresh() {
  if (running) {
    screens.Refresh();
  }
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
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_fmt(label,
                        "#FFFF00 InfiniTime#\n\n"
                        "#444444 Version# %ld.%ld.%ld\n\n"
                        "#444444 Short Ref# %s\n\n"
                        "#444444 Build date#\n"
                        "%s\n"
                        "%s\n",
                        Version::Major(),
                        Version::Minor(),
                        Version::Patch(),
                        Version::GitCommitHash(),
                        __DATE__,
                        __TIME__);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(0, 5, app, label));
}

std::unique_ptr<Screen> SystemInfo::CreateScreen2() {
  auto batteryPercent = static_cast<uint8_t>(batteryController.PercentRemaining());
  float batteryVoltage = batteryController.Voltage();

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

  if (batteryPercent == -1)
    batteryPercent = 0;

  // hack to not use the flot functions from printf
  uint8_t batteryVoltageBytes[2];
  batteryVoltageBytes[1] = static_cast<uint8_t>(batteryVoltage); // truncate whole numbers
  batteryVoltageBytes[0] =
    static_cast<uint8_t>((batteryVoltage - batteryVoltageBytes[1]) * 100); // remove whole part of flt and shift 2 places over
  //

  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_fmt(label,
                        "#444444 Date# %02d/%02d/%04d\n"
                        "#444444 Time# %02d:%02d:%02d\n"
                        "#444444 Uptime#\n %02lud %02lu:%02lu:%02lu\n"
                        "#444444 Battery# %d%%/%1i.%02iv\n"
                        "#444444 Backlight# %s\n"
                        "#444444 Last reset# %s\n",
                        dateTimeController.Day(),
                        static_cast<uint8_t>(dateTimeController.Month()),
                        dateTimeController.Year(),
                        dateTimeController.Hours(),
                        dateTimeController.Minutes(),
                        dateTimeController.Seconds(),
                        uptimeDays,
                        uptimeHours,
                        uptimeMinutes,
                        uptimeSeconds,
                        batteryPercent,
                        batteryVoltageBytes[1],
                        batteryVoltageBytes[0],
                        brightnessController.ToString(),
                        resetReason);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(1, 4, app, label));
}

std::unique_ptr<Screen> SystemInfo::CreateScreen3() {
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);

  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  auto& bleAddr = bleController.Address();
  lv_label_set_text_fmt(label,
                        "#444444 BLE MAC#\n"
                        " %02x:%02x:%02x:%02x:%02x:%02x"
                        "\n"
                        "#444444 LVGL Memory#\n"
                        " #444444 used# %d (%d%%)\n"
                        " #444444 max used# %d\n"
                        " #444444 frag# %d%%\n"
                        " #444444 free# %d"
                        "\n"
                        "#444444 Steps# %li",
                        bleAddr[5],
                        bleAddr[4],
                        bleAddr[3],
                        bleAddr[2],
                        bleAddr[1],
                        bleAddr[0],
                        (int) mon.total_size - mon.free_size,
                        mon.used_pct,
                        mon.max_used,
                        mon.frag_pct,
                        (int) mon.free_biggest_size,
                        0);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(2, 5, app, label));
}

bool sortById(const TaskStatus_t& lhs, const TaskStatus_t& rhs) {
  return lhs.xTaskNumber < rhs.xTaskNumber;
}

std::unique_ptr<Screen> SystemInfo::CreateScreen4() {
  TaskStatus_t tasksStatus[7];
  lv_obj_t* infoTask = lv_table_create(lv_scr_act(), NULL);
  lv_table_set_col_cnt(infoTask, 3);
  lv_table_set_row_cnt(infoTask, 8);
  lv_obj_set_pos(infoTask, 10, 10);

  lv_table_set_cell_value(infoTask, 0, 0, "#");
  lv_table_set_col_width(infoTask, 0, 50);
  lv_table_set_cell_value(infoTask, 0, 1, "Task");
  lv_table_set_col_width(infoTask, 1, 80);
  lv_table_set_cell_value(infoTask, 0, 2, "Free");
  lv_table_set_col_width(infoTask, 2, 90);

  auto nb = uxTaskGetSystemState(tasksStatus, 7, nullptr);
  std::sort(tasksStatus, tasksStatus + nb, sortById);
  for (uint8_t i = 0; i < nb; i++) {

    lv_table_set_cell_value(infoTask, i + 1, 0, std::to_string(tasksStatus[i].xTaskNumber).c_str());
    lv_table_set_cell_value(infoTask, i + 1, 1, tasksStatus[i].pcTaskName);
    if (tasksStatus[i].usStackHighWaterMark < 20) {
      std::string str1 = std::to_string(tasksStatus[i].usStackHighWaterMark) + " low";
      lv_table_set_cell_value(infoTask, i + 1, 2, str1.c_str());
    } else {
      lv_table_set_cell_value(infoTask, i + 1, 2, std::to_string(tasksStatus[i].usStackHighWaterMark).c_str());
    }
  }
  return std::unique_ptr<Screen>(new Screens::Label(3, 5, app, infoTask));
}

std::unique_ptr<Screen> SystemInfo::CreateScreen5() {
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_static(label,
                           "Software Licensed\n"
                           "under the terms of\n"
                           "the GNU General\n"
                           "Public License v3\n"
                           "#444444 Source code#\n"
                           "#FFFF00 https://github.com/#\n"
                           "#FFFF00 JF002/InfiniTime#");
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(4, 5, app, label));
}
