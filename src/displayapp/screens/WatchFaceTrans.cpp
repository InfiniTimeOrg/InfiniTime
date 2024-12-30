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
  lv_obj_set_style_local_bg_color(topBlueBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightBlue);
  lv_obj_set_style_local_radius(topBlueBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  topPinkBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(topPinkBackground, LV_HOR_RES, LV_VER_RES / 5);
  lv_obj_set_pos(topPinkBackground, 0, LV_VER_RES / 5);
  lv_obj_set_style_local_bg_color(topPinkBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightPink);
  lv_obj_set_style_local_radius(topPinkBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  whiteBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(whiteBackground, LV_HOR_RES, LV_VER_RES / 5);
  lv_obj_set_pos(whiteBackground, 0, 2 * LV_VER_RES / 5);
  lv_obj_set_style_local_bg_color(whiteBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_radius(whiteBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  bottomPinkBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(bottomPinkBackground, LV_HOR_RES, LV_VER_RES / 5);
  lv_obj_set_pos(bottomPinkBackground, 0, 3 * LV_VER_RES / 5);
  lv_obj_set_style_local_bg_color(bottomPinkBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightPink);
  lv_obj_set_style_local_radius(bottomPinkBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  bottomBlueBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(bottomBlueBackground, LV_HOR_RES, LV_VER_RES / 5);
  lv_obj_set_pos(bottomBlueBackground, 0, 4 * LV_VER_RES / 5);
  lv_obj_set_style_local_bg_color(bottomBlueBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightBlue);
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
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelDate, true);
  lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_CENTER, 0, -48);
  lv_label_set_align(labelDate, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(labelDate, true);

  labelTime = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelTime, true);
  lv_obj_align(labelTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_label_set_align(labelTime, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_style_local_text_font(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_auto_realign(labelTime, true);

  labelDay = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelDay, true);
  lv_obj_align(labelDay, lv_scr_act(), LV_ALIGN_CENTER, 0, 48);
  lv_label_set_align(labelDay, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(labelDay, true);

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
    lv_label_set_text_fmt(batteryValue, "#ffffff %d%%#", batteryPercentRemaining.Get());
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValue, LV_LABEL_POS_LAST, " Charging");
    }
  }
  if (bleState.IsUpdated()) {
    if (bleState.Get()) {
      lv_label_set_text_static(bluetoothStatus, Symbols::bluetooth);
    } else {
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
      lv_label_set_text_fmt(labelTime, "#000000 %02d:%02d:%02d %s#", hour, minute, second, ampmChar);
    } else {
      lv_label_set_text_fmt(labelTime, "#000000 %02d:%02d:%02d#", hour, minute, second);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      Controllers::DateTime::Days dayOfWeek = dateTimeController.DayOfWeek();
      lv_label_set_text_fmt(labelDate, "#ffffff %02d-%02d-%04d#", day, static_cast<uint8_t>(month), year);
      lv_label_set_text_fmt(labelDay, "#ffffff %s#", dateTimeController.DayOfWeekToStringLow(dayOfWeek));
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "#ffffff %lu steps#", stepCount.Get());
  }
}
