#include "displayapp/screens/WatchFaceNumerals.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/Symbols.h"
#include "components/ble/NotificationManager.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

WatchFaceNumerals::WatchFaceNumerals(Controllers::DateTime& dateTimeController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::FS& filesystem)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
    settingsController {settingsController} {

  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/rounded_large.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_large = lv_font_load("F:/fonts/rounded_large.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/rounded_small.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_small = lv_font_load("F:/fonts/rounded_small.bin");
  }

  notificationIcon = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(notificationIcon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_radius(notificationIcon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_size(notificationIcon, 14, 14);
  lv_obj_align(notificationIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 2, -72);
  lv_obj_set_hidden(notificationIcon, true);

  labelTimeHour = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelTimeHour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_large);
  lv_obj_set_style_local_text_color(labelTimeHour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_align(labelTimeHour, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -160, -125);

  labelTimeMinute = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelTimeMinute, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_large);
  lv_obj_set_style_local_text_color(labelTimeMinute, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_align(labelTimeMinute, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -160, 0);

  labelTimeAMPM1 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelTimeAMPM1, "");
  lv_obj_set_style_local_text_font(labelTimeAMPM1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_small);
  lv_obj_set_style_local_text_color(labelTimeAMPM1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_align(labelTimeAMPM1, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, -32);

  labelTimeAMPM2 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelTimeAMPM2, "M");
  lv_obj_set_style_local_text_font(labelTimeAMPM2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_small);
  lv_obj_set_style_local_text_color(labelTimeAMPM2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_align(labelTimeAMPM2, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  dateDayOfWeek = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateDayOfWeek, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text(dateDayOfWeek, "---");
  lv_obj_set_style_local_text_font(dateDayOfWeek, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_small);
  lv_obj_align(dateDayOfWeek, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  dateDay = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateDay, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text(dateDay, "--");
  lv_obj_set_style_local_text_font(dateDay, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_small);
  lv_obj_align(dateDay, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 34);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceNumerals::~WatchFaceNumerals() {
  lv_task_del(taskRefresh);

  if (font_large != nullptr) {
    lv_font_free(font_large);
  }
  if (font_small != nullptr) {
    lv_font_free(font_small);
  }

  lv_obj_clean(lv_scr_act());
}

void WatchFaceNumerals::Refresh() {
  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_obj_set_hidden(notificationIcon, !notificationState.Get());
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    auto hour = dateTimeController.Hours();
    auto minute = dateTimeController.Minutes();
    auto year = dateTimeController.Year();
    auto month = dateTimeController.Month();
    auto dayOfWeek = dateTimeController.DayOfWeek();
    auto day = dateTimeController.Day();

    if (displayedHour != hour || displayedMinute != minute) {
      displayedHour = hour;
      displayedMinute = minute;

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        char ampmChar[2] = "A";
        if (hour == 0) {
          hour = 12;
        } else if (hour == 12) {
          ampmChar[0] = 'P';
        } else if (hour > 12) {
          hour = hour - 12;
          ampmChar[0] = 'P';
        }
        lv_label_set_text(labelTimeAMPM1, ampmChar);
        lv_label_set_text_fmt(labelTimeHour, "%02d", hour);
        lv_label_set_text_fmt(labelTimeMinute, "%02d", minute);
      } else {
        lv_obj_set_hidden(labelTimeAMPM2, true);
        lv_label_set_text_fmt(labelTimeHour, "%02d", hour);
        lv_label_set_text_fmt(labelTimeMinute, "%02d", minute);
      }
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_static(dateDayOfWeek, dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(dateDay, "%d", day);
      lv_obj_realign(dateDay);

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }
}

bool WatchFaceNumerals::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/rounded_small.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/rounded_large.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  return true;
}