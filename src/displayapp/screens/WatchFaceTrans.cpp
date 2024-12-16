#include <lvgl/lvgl.h>
#include "displayapp/screens/WatchFaceTrans.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

WatchFaceTrans::WatchFaceTrans(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::MotionController& motionController)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController} {

  topBlueBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(topBlueBackground, LV_HOR_RES, LV_VER_RES / 5);
  lv_obj_set_pos(topBlueBackground, 0, 0);
  lv_obj_set_style_local_bg_color(topBlueBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00bff3));
  lv_obj_set_style_local_radius(topBlueBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  topPinkBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(topPinkBackground, LV_HOR_RES, LV_VER_RES / 5);
  lv_obj_set_pos(topPinkBackground, 0, LV_VER_RES / 5);
  lv_obj_set_style_local_bg_color(topPinkBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xf49ac1));
  lv_obj_set_style_local_radius(topPinkBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  whiteBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(whiteBackground, LV_HOR_RES, LV_VER_RES / 5);
  lv_obj_set_pos(whiteBackground, 0, 2 * LV_VER_RES / 5);
  lv_obj_set_style_local_bg_color(whiteBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
  lv_obj_set_style_local_radius(whiteBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  bottomPinkBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(bottomPinkBackground, LV_HOR_RES, LV_VER_RES / 5);
  lv_obj_set_pos(bottomPinkBackground, 0, 3 * LV_VER_RES / 5);
  lv_obj_set_style_local_bg_color(bottomPinkBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xf49ac1));
  lv_obj_set_style_local_radius(bottomPinkBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  bottomBlueBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(bottomBlueBackground, LV_HOR_RES, LV_VER_RES / 5);
  lv_obj_set_pos(bottomBlueBackground, 0, 4 * LV_VER_RES / 5);
  lv_obj_set_style_local_bg_color(bottomBlueBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00bff3));
  lv_obj_set_style_local_radius(bottomBlueBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

  bluetoothStatus = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bluetoothStatus, "");
  lv_obj_align(bluetoothStatus, nullptr, LV_ALIGN_IN_TOP_RIGHT, -16, 0);

  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(batteryValue, true);
  lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -96);
  lv_label_set_align(batteryValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(batteryValue, true);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_LEFT_MID, 0, -100);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_date, true);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, -48);
  lv_label_set_align(label_date, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(label_date, true);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_time, true);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_auto_realign(label_time, true);

  label_day = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_day, true);
  lv_obj_align(label_day, lv_scr_act(), LV_ALIGN_CENTER, 0, 48);
  lv_label_set_align(label_day, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(label_day, true);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(stepValue, true);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_CENTER, 0, 96);
  lv_label_set_align(stepValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(stepValue, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceTrans::~WatchFaceTrans() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceTrans::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  bleState = bleController.IsConnected();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    lv_label_set_text_fmt(batteryValue, "#ffffff %d%%", batteryPercentRemaining.Get());
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValue, LV_LABEL_POS_LAST, " Charging");
    }
  }
  if (bleState.IsUpdated()) {
    if (bleState.Get()) {
      lv_label_set_text_static(bluetoothStatus, Symbols::bluetooth);
    }
    else {
      lv_label_set_text_static(bluetoothStatus, "");
    }
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (notificationState.Get()) {
      lv_label_set_text_static(notificationIcon, "You have mail.");
    } else {
      lv_label_set_text_static(notificationIcon, "");
    }
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    uint8_t second = dateTimeController.Seconds();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text_fmt(label_time, "#000000 %02d:%02d:%02d %s#", hour, minute, second, ampmChar);
    } else {
      lv_label_set_text_fmt(label_time, "#000000 %02d:%02d:%02d", hour, minute, second);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      Controllers::DateTime::Days dayOfWeek = dateTimeController.DayOfWeek();
      lv_label_set_text_fmt(label_date, "#ffffff %02d-%02d-%04d#", short(day), char(month), year);
      const char* dayString;
      switch (dayOfWeek) {
        case Controllers::DateTime::Days::Monday:
          dayString = "Monday";
          break;
        case Controllers::DateTime::Days::Tuesday:
          dayString = "Tuesday";
          break;
        case Controllers::DateTime::Days::Wednesday:
          dayString = "Wednesday";
          break;
        case Controllers::DateTime::Days::Thursday:
          dayString = "Thursday";
          break;
        case Controllers::DateTime::Days::Friday:
          dayString = "Friday";
          break;
        case Controllers::DateTime::Days::Saturday:
          dayString = "Saturday";
          break;
        case Controllers::DateTime::Days::Sunday:
          dayString = "Sunday";
          break;
        default:
          dayString = "?";
          break;
      }
      lv_label_set_text_fmt(label_day, "#ffffff %s", dayString);
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "#ffffff %lu steps#", stepCount.Get());
  }
}
