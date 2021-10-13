/*
 * This file is part of the Infinitime distribution (https://github.com/JF002/Infinitime).
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

  // This sets the watchface number to return to after leaving the menu
  settingsController.SetClockFace(2);

  displayedChar[0] = 0;
  displayedChar[1] = 0;
  displayedChar[2] = 0;
  displayedChar[3] = 0;
  displayedChar[4] = 0;

  //Create a 200px wide background rectangle
  timebar = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(timebar, Convert(settingsController.GetPTSColorBG()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(timebar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(timebar, 200, 240);
  lv_obj_align(timebar, LV_ALIGN_TOP_LEFT, 5, 0);

  // Display the time
  timeDD1 = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(timeDD1, &open_sans_light, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(timeDD1, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(timeDD1, "12");
  lv_obj_align_to(timeDD1, timebar, LV_ALIGN_TOP_MID, 5, 5);

  timeDD2 = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(timeDD2, &open_sans_light, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(timeDD2, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(timeDD2, "34");
  lv_obj_align_to(timeDD2, timebar, LV_ALIGN_BOTTOM_MID, 5, -5);

  timeAMPM = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(timeAMPM, Convert(settingsController.GetPTSColorTime()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_line_space(timeAMPM, -3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(timeAMPM, "");
  lv_obj_align_to(timeAMPM, timebar, LV_ALIGN_BOTTOM_LEFT, 2, -20);

  // Create a 40px wide bar down the right side of the screen
  sidebar = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(sidebar, Convert(settingsController.GetPTSColorBar()), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(sidebar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(sidebar, 40, 240);
  lv_obj_align(sidebar, LV_ALIGN_TOP_RIGHT, 0, 0);

  // Display icons
  batteryIcon = lv_label_create(sidebar);
  lv_obj_set_style_text_color(batteryIcon, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, LV_ALIGN_TOP_MID, 0, 2);

  bleIcon = lv_label_create(sidebar);
  lv_obj_set_style_text_color(bleIcon, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(bleIcon, LV_ALIGN_TOP_MID, 0, 25);

  notificationIcon = lv_label_create(sidebar);
  lv_obj_set_style_text_color(notificationIcon, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(notificationIcon, LV_ALIGN_TOP_MID, 0, 40);

  // Calendar icon
  calendarOuter = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarOuter, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarOuter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarOuter, 34, 34);
  lv_obj_align_to(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 0);

  calendarInner = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarInner, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarInner, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarInner, 27, 27);
  lv_obj_align_to(calendarInner, calendarOuter, LV_ALIGN_CENTER, 0, 0);

  calendarBar1 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarBar1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarBar1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarBar1, 3, 12);
  lv_obj_align_to(calendarBar1, calendarOuter, LV_ALIGN_TOP_MID, -6, -3);

  calendarBar2 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarBar2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarBar2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarBar2, 3, 12);
  lv_obj_align_to(calendarBar2, calendarOuter, LV_ALIGN_TOP_MID, 6, -3);

  calendarCrossBar1 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarCrossBar1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarCrossBar1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarCrossBar1, 8, 3);
  lv_obj_align_to(calendarCrossBar1, calendarBar1, LV_ALIGN_BOTTOM_MID, 0, 0);

  calendarCrossBar2 = lv_obj_create(lv_scr_act());
  lv_obj_set_style_bg_color(calendarCrossBar2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(calendarCrossBar2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(calendarCrossBar2, 8, 3);
  lv_obj_align_to(calendarCrossBar2, calendarBar2, LV_ALIGN_BOTTOM_MID, 0, 0);

  // Display date
  dateDayOfWeek = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(dateDayOfWeek, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(dateDayOfWeek, "THU");
  lv_obj_align_to(dateDayOfWeek, sidebar, LV_ALIGN_CENTER, 0, -34);

  dateDay = lv_label_create(calendarInner);
  lv_obj_set_style_text_color(dateDay, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(dateDay, "25");
  lv_obj_center(dateDay);

  dateMonth = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(dateMonth, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(dateMonth, "MAR");
  lv_obj_align_to(dateMonth, sidebar, LV_ALIGN_CENTER, 0, 32);

  // Step count gauge
  stepCont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(stepCont, 40, 40);
  lv_obj_align(stepCont, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  
  needle_color = lv_color_white();
  stepMeter = lv_meter_create(stepCont);
  lv_obj_set_size(stepMeter, 37, 37);
  lv_obj_center(stepMeter);

  stepScale = lv_meter_add_scale(stepMeter);
  stepIndicator = lv_meter_add_needle_line(stepMeter, stepScale, 2, needle_color, -6);
  lv_meter_set_scale_range(stepMeter, stepScale, 0, 100, 360, 180);
  lv_meter_set_scale_ticks(stepMeter, stepScale, 11, 4, 4, lv_color_black());
  lv_meter_set_indicator_value(stepMeter, stepIndicator, 0);

  lv_obj_set_style_pad_all(stepMeter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(stepMeter, LV_OPA_TRANSP, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(stepMeter, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_line_color(stepMeter, lv_color_black(), LV_PART_TICKS | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(stepCont, LV_OPA_TRANSP, 0);

  backgroundLabel = lv_label_create(lv_scr_act());
  lv_obj_add_flag(backgroundLabel, LV_OBJ_FLAG_CLICKABLE);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, this);
  Refresh();
}

PineTimeStyle::~PineTimeStyle() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
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
  }

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
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
    lv_meter_set_indicator_value(stepMeter, stepIndicator, (stepCount.Get() / (settingsController.GetStepsGoal() / 100)));
    if (stepCount.Get() > settingsController.GetStepsGoal()) {
      lv_obj_set_style_line_color(stepMeter, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_grad_color(stepMeter, lv_color_white(), LV_PART_TICKS | LV_STATE_DEFAULT);
    }
  }
}
