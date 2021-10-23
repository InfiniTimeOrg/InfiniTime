/*
 * This file is part of the Infinitime distribution (https://github.com/InfiniTimeOrg/Infinitime).
 * Copyright (c) 2021 Kieran Cawthray.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * PineTimeStyle watchface for Infinitime created by Kieran Cawthray
 * Based on WatchFaceDigital
 * Style/layout copied from TimeStyle for Pebble by Dan Tilden (github.com/tilden)
 */

#include "PineTimeStyle.h"
#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include <displayapp/Colors.h>
#include "BatteryIcon.h"
#include "BleIcon.h"
#include "NotificationIcon.h"
#include "Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    PineTimeStyle* screen = static_cast<PineTimeStyle*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

PineTimeStyle::PineTimeStyle(DisplayApp* app,
                             Controllers::DateTime& dateTimeController,
                             Controllers::Battery& batteryController,
                             Controllers::Ble& bleController,
                             Controllers::NotificationManager& notificatioManager,
                             Controllers::Settings& settingsController,
                             Controllers::MotionController& motionController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    motionController {motionController} {

  displayedChar[0] = 0;
  displayedChar[1] = 0;
  displayedChar[2] = 0;
  displayedChar[3] = 0;
  displayedChar[4] = 0;

  // Create a 200px wide background rectangle
  timebar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorBG()));
  lv_obj_set_style_local_radius(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(timebar, 200, 240);
  lv_obj_align(timebar, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 0);

  // Display the time
  timeDD1 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &open_sans_light);
  lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_label_set_text(timeDD1, "12");
  lv_obj_align(timeDD1, timebar, LV_ALIGN_IN_TOP_MID, 5, 5);

  timeDD2 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &open_sans_light);
  lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_label_set_text(timeDD2, "34");
  lv_obj_align(timeDD2, timebar, LV_ALIGN_IN_BOTTOM_MID, 5, -5);

  timeAMPM = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_obj_set_style_local_text_line_space(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, -3);
  lv_label_set_text(timeAMPM, "");
  lv_obj_align(timeAMPM, timebar, LV_ALIGN_IN_BOTTOM_LEFT, 2, -20);

  // Create a 40px wide bar down the right side of the screen
  sidebar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorBar()));
  lv_obj_set_style_local_radius(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(sidebar, 40, 240);
  lv_obj_align(sidebar, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  // Display icons
  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, sidebar, LV_ALIGN_IN_TOP_MID, 0, 2);
  lv_obj_set_auto_realign(batteryIcon, true);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_align(bleIcon, sidebar, LV_ALIGN_IN_TOP_MID, 0, 25);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_align(notificationIcon, sidebar, LV_ALIGN_IN_TOP_MID, 0, 40);

  // Calendar icon
  calendarOuter = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarOuter, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_radius(calendarOuter, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarOuter, 34, 34);
  lv_obj_align(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 0);

  calendarInner = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarInner, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_radius(calendarInner, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarInner, 27, 27);
  lv_obj_align(calendarInner, calendarOuter, LV_ALIGN_CENTER, 0, 0);

  calendarBar1 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarBar1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_radius(calendarBar1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarBar1, 3, 12);
  lv_obj_align(calendarBar1, calendarOuter, LV_ALIGN_IN_TOP_MID, -6, -3);

  calendarBar2 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarBar2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_radius(calendarBar2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarBar2, 3, 12);
  lv_obj_align(calendarBar2, calendarOuter, LV_ALIGN_IN_TOP_MID, 6, -3);

  calendarCrossBar1 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarCrossBar1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_radius(calendarCrossBar1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarCrossBar1, 8, 3);
  lv_obj_align(calendarCrossBar1, calendarBar1, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  calendarCrossBar2 = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarCrossBar2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_radius(calendarCrossBar2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarCrossBar2, 8, 3);
  lv_obj_align(calendarCrossBar2, calendarBar2, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  // Display date
  dateDayOfWeek = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateDayOfWeek, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text(dateDayOfWeek, "THU");
  lv_obj_align(dateDayOfWeek, sidebar, LV_ALIGN_CENTER, 0, -34);

  dateDay = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateDay, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text(dateDay, "25");
  lv_obj_align(dateDay, sidebar, LV_ALIGN_CENTER, 0, 3);

  dateMonth = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateMonth, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text(dateMonth, "MAR");
  lv_obj_align(dateMonth, sidebar, LV_ALIGN_CENTER, 0, 32);

  // Step count gauge
  if (settingsController.GetPTSColorBar() == Pinetime::Controllers::Settings::Colors::White) {
    needle_colors[0] = LV_COLOR_BLACK; 
    } else {
    needle_colors[0] = LV_COLOR_WHITE;
  }
  stepGauge = lv_gauge_create(lv_scr_act(), nullptr);
  lv_gauge_set_needle_count(stepGauge, 1, needle_colors);
  lv_obj_set_size(stepGauge, 40, 40);
  lv_obj_align(stepGauge, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_gauge_set_scale(stepGauge, 360, 11, 0);
  lv_gauge_set_angle_offset(stepGauge, 180);
  lv_gauge_set_critical_value(stepGauge, 100);
  lv_gauge_set_range(stepGauge, 0, 100);
  lv_gauge_set_value(stepGauge, 0, 0);

  lv_obj_set_style_local_pad_right(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_pad_left(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_pad_bottom(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_line_opa(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_scale_width(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_color(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_line_opa(stepGauge, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_line_width(stepGauge, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_pad_inner(stepGauge, LV_GAUGE_PART_NEEDLE, LV_STATE_DEFAULT, 4);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  btnNextTime = lv_btn_create(lv_scr_act(), nullptr);
  btnNextTime->user_data = this;
  lv_obj_set_size(btnNextTime, 60, 60);
  lv_obj_align(btnNextTime, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, -80);
  lv_obj_set_style_local_bg_opa(btnNextTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnNextTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, ">");
  lv_obj_set_event_cb(btnNextTime, event_handler);
  lv_obj_set_hidden(btnNextTime, true);

  btnPrevTime = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevTime->user_data = this;
  lv_obj_set_size(btnPrevTime, 60, 60);
  lv_obj_align(btnPrevTime, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, -80);
  lv_obj_set_style_local_bg_opa(btnPrevTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnPrevTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<");
  lv_obj_set_event_cb(btnPrevTime, event_handler);
  lv_obj_set_hidden(btnPrevTime, true);

  btnNextBar = lv_btn_create(lv_scr_act(), nullptr);
  btnNextBar->user_data = this;
  lv_obj_set_size(btnNextBar, 60, 60);
  lv_obj_align(btnNextBar, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 0);
  lv_obj_set_style_local_bg_opa(btnNextBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnNextBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, ">");
  lv_obj_set_event_cb(btnNextBar, event_handler);
  lv_obj_set_hidden(btnNextBar, true);

  btnPrevBar = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevBar->user_data = this;
  lv_obj_set_size(btnPrevBar, 60, 60);
  lv_obj_align(btnPrevBar, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 0);
  lv_obj_set_style_local_bg_opa(btnPrevBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnPrevBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<");
  lv_obj_set_event_cb(btnPrevBar, event_handler);
  lv_obj_set_hidden(btnPrevBar, true);

  btnNextBG = lv_btn_create(lv_scr_act(), nullptr);
  btnNextBG->user_data = this;
  lv_obj_set_size(btnNextBG, 60, 60);
  lv_obj_align(btnNextBG, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 80);
  lv_obj_set_style_local_bg_opa(btnNextBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnNextBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, ">");
  lv_obj_set_event_cb(btnNextBG, event_handler);
  lv_obj_set_hidden(btnNextBG, true);

  btnPrevBG = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevBG->user_data = this;
  lv_obj_set_size(btnPrevBG, 60, 60);
  lv_obj_align(btnPrevBG, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 80);
  lv_obj_set_style_local_bg_opa(btnPrevBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnPrevBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "<");
  lv_obj_set_event_cb(btnPrevBG, event_handler);
  lv_obj_set_hidden(btnPrevBG, true);

  btnReset = lv_btn_create(lv_scr_act(), nullptr);
  btnReset->user_data = this;
  lv_obj_set_size(btnReset, 60, 60);
  lv_obj_align(btnReset, lv_scr_act(), LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_local_bg_opa(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Rst");
  lv_obj_set_event_cb(btnReset, event_handler);
  lv_obj_set_hidden(btnReset, true);

  btnRandom = lv_btn_create(lv_scr_act(), nullptr);
  btnRandom->user_data = this;
  lv_obj_set_size(btnRandom, 60, 60);
  lv_obj_align(btnRandom, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(btnRandom, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnRandom, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Rnd");
  lv_obj_set_event_cb(btnRandom, event_handler);
  lv_obj_set_hidden(btnRandom, true);

  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_value_str(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  btnSet = lv_btn_create(lv_scr_act(), nullptr);
  btnSet->user_data = this;
  lv_obj_set_height(btnSet, 150);
  lv_obj_set_width(btnSet, 150);
  lv_obj_align(btnSet, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_radius(btnSet, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 30);
  lv_obj_set_style_local_bg_opa(btnSet, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_event_cb(btnSet, event_handler);
  lbl_btnSet = lv_label_create(btnSet, nullptr);
  lv_obj_set_style_local_text_font(lbl_btnSet, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(lbl_btnSet, Symbols::settings);
  lv_obj_set_hidden(btnSet, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

PineTimeStyle::~PineTimeStyle() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

bool PineTimeStyle::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && (lv_obj_get_hidden(btnRandom) == true)) {
    lv_obj_set_hidden(btnSet, false);
    savedTick = lv_tick_get();
    return true;
  }
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && (lv_obj_get_hidden(btnRandom) == false)) {
    return true;
  }
  return false;
}

void PineTimeStyle::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));
}

void PineTimeStyle::Refresh() {
  isCharging = batteryController.IsCharging();
  if (isCharging.IsUpdated()) {
    if (isCharging.Get()) {
      lv_label_set_text(batteryIcon, Symbols::plug);
    } else {
      SetBatteryIcon();
    }
  }
  if (!isCharging.Get()) {
    batteryPercentRemaining = batteryController.PercentRemaining();
    if (batteryPercentRemaining.IsUpdated()) {
      SetBatteryIcon();
    }
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    lv_label_set_text(bleIcon, BleIcon::GetIcon(bleState.Get()));
    lv_obj_realign(bleIcon);
  }

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
    lv_obj_realign(notificationIcon);
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime - dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto year = static_cast<int>(yearMonthDay.year());
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>(static_cast<unsigned>(yearMonthDay.month()));
    auto day = static_cast<unsigned>(yearMonthDay.day());
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    int hour = time.hours().count();
    auto minute = time.minutes().count();

    char minutesChar[3];
    sprintf(minutesChar, "%02d", static_cast<int>(minute));

    char hoursChar[3];
    char ampmChar[5];
    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        sprintf(hoursChar, "%02d", hour);
    } else {
      if (hour == 0 && hour != 12) {
        hour = 12;
        sprintf(ampmChar, "A\nM");
      } else if (hour == 12 && hour != 0) {
        hour = 12;
        sprintf(ampmChar, "P\nM");
      } else if (hour < 12 && hour != 0) {
        sprintf(ampmChar, "A\nM");
      } else if (hour > 12 && hour != 0) {
        hour = hour - 12;
        sprintf(ampmChar, "P\nM");
      }
      sprintf(hoursChar, "%02d", hour);
    }

    if (hoursChar[0] != displayedChar[0] or hoursChar[1] != displayedChar[1] or minutesChar[0] != displayedChar[2] or
        minutesChar[1] != displayedChar[3]) {
      displayedChar[0] = hoursChar[0];
      displayedChar[1] = hoursChar[1];
      displayedChar[2] = minutesChar[0];
      displayedChar[3] = minutesChar[1];

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        lv_label_set_text(timeAMPM, ampmChar);
      }

      lv_label_set_text_fmt(timeDD1, "%s", hoursChar);
      lv_label_set_text_fmt(timeDD2, "%s", minutesChar);
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_fmt(dateDayOfWeek, "%s", dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(dateDay, "%d", day);
      lv_obj_realign(dateDay);
      lv_label_set_text_fmt(dateMonth, "%s", dateTimeController.MonthShortToString());

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_gauge_set_value(stepGauge, 0, (stepCount.Get() / (settingsController.GetStepsGoal() / 100)));
    lv_obj_realign(stepGauge);
    if (stepCount.Get() > settingsController.GetStepsGoal()) {
      lv_obj_set_style_local_line_color(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_scale_grad_color(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    }
  }
  if (lv_obj_get_hidden(btnSet) == false) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      lv_obj_set_hidden(btnSet, true);
      savedTick = 0;
    }
  }
}

void PineTimeStyle::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  auto valueTime = settingsController.GetPTSColorTime();
  auto valueBar = settingsController.GetPTSColorBar();
  auto valueBG = settingsController.GetPTSColorBG();

  if (event == LV_EVENT_CLICKED) {
    if (object == btnNextTime) {
      valueTime = GetNext(valueTime);
      if(valueTime == valueBG)
        valueTime = GetNext(valueTime);
      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
    }
    if (object == btnPrevTime) {
      valueTime = GetPrevious(valueTime);
      if(valueTime == valueBG)
        valueTime = GetPrevious(valueTime);
      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
    }
    if (object == btnNextBar) {
      valueBar = GetNext(valueBar);
      if(valueBar == Controllers::Settings::Colors::Black)
        valueBar = GetNext(valueBar);
      if(valueBar == Controllers::Settings::Colors::White) {
        needle_colors[0] = LV_COLOR_BLACK; 
        } else {
        needle_colors[0] = LV_COLOR_WHITE;
      }
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBar));
    }
    if (object == btnPrevBar) {
      valueBar = GetPrevious(valueBar);
      if(valueBar == Controllers::Settings::Colors::Black)
        valueBar = GetPrevious(valueBar);
      if(valueBar == Controllers::Settings::Colors::White) {
        needle_colors[0] = LV_COLOR_BLACK; 
        } else {
        needle_colors[0] = LV_COLOR_WHITE;
      }
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBar));
    }
    if (object == btnNextBG) {
      valueBG = GetNext(valueBG);
      if(valueBG == valueTime)
        valueBG = GetNext(valueBG);
      settingsController.SetPTSColorBG(valueBG);
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBG));
    }
    if (object == btnPrevBG) {
      valueBG = GetPrevious(valueBG);
      if(valueBG == valueTime)
        valueBG = GetPrevious(valueBG);
      settingsController.SetPTSColorBG(valueBG);
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBG));
    }
    if (object == btnReset) {
      needle_colors[0] = LV_COLOR_WHITE;
      settingsController.SetPTSColorTime(Controllers::Settings::Colors::Teal);
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Teal));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Teal));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Teal));
      settingsController.SetPTSColorBar(Controllers::Settings::Colors::Teal);
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Teal));
      settingsController.SetPTSColorBG(Controllers::Settings::Colors::Black);
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Black));
    }
    if (object == btnRandom) {
      valueTime = static_cast<Controllers::Settings::Colors>(rand() % 17);
      valueBar = static_cast<Controllers::Settings::Colors>(rand() % 17);
      valueBG = static_cast<Controllers::Settings::Colors>(rand() % 17);
      if (valueTime == valueBG) {
        valueBG = GetNext(valueBG);
      }
      if (valueBar == Controllers::Settings::Colors::Black) {
        valueBar = GetPrevious(valueBar);
      }
      if (valueBar == Controllers::Settings::Colors::White) {
        needle_colors[0] = LV_COLOR_BLACK;
      } else {
        needle_colors[0] = LV_COLOR_WHITE;
      }
      settingsController.SetPTSColorTime(static_cast<Controllers::Settings::Colors>(valueTime));
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,  Convert(valueTime));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT,  Convert(valueTime));
      settingsController.SetPTSColorBar(static_cast<Controllers::Settings::Colors>(valueBar));
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT,  Convert(valueBar));
      settingsController.SetPTSColorBG(static_cast<Controllers::Settings::Colors>(valueBG));
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT,  Convert(valueBG));
    }
    if (object == btnClose) {
      settingsController.SaveSettings();
      lv_obj_set_hidden(btnNextTime, true);
      lv_obj_set_hidden(btnPrevTime, true);
      lv_obj_set_hidden(btnNextBar, true);
      lv_obj_set_hidden(btnPrevBar, true);
      lv_obj_set_hidden(btnNextBG, true);
      lv_obj_set_hidden(btnPrevBG, true);
      lv_obj_set_hidden(btnReset, true);
      lv_obj_set_hidden(btnRandom, true);
      lv_obj_set_hidden(btnClose, true);
    }
    if (object == btnSet) {
      lv_obj_set_hidden(btnSet, true);
      lv_obj_set_hidden(btnNextTime, false);
      lv_obj_set_hidden(btnPrevTime, false);
      lv_obj_set_hidden(btnNextBar, false);
      lv_obj_set_hidden(btnPrevBar, false);
      lv_obj_set_hidden(btnNextBG, false);
      lv_obj_set_hidden(btnPrevBG, false);
      lv_obj_set_hidden(btnReset, false);
      lv_obj_set_hidden(btnRandom, false);
      lv_obj_set_hidden(btnClose, false);
    }
  }
}

Pinetime::Controllers::Settings::Colors PineTimeStyle::GetNext(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors nextColor;
  if (colorAsInt < 16) {
    nextColor = static_cast<Controllers::Settings::Colors>(colorAsInt + 1);
  } else {
    nextColor = static_cast<Controllers::Settings::Colors>(0);
  }
  return nextColor;
}

Pinetime::Controllers::Settings::Colors PineTimeStyle::GetPrevious(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors prevColor;

  if (colorAsInt > 0) {
    prevColor = static_cast<Controllers::Settings::Colors>(colorAsInt - 1);
  } else {
    prevColor = static_cast<Controllers::Settings::Colors>(16);
  }
  return prevColor;
}