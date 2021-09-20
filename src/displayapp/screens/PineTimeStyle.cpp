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
  : PineTimeStyleBase(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    motionController {motionController} {

  // This sets the watchface number to return to after leaving the menu
  settingsController.SetClockFace(2);

  CreateObjects(Convert(settingsController.GetPTSColorBG()), Convert(settingsController.GetPTSColorTime()), Convert(settingsController.GetPTSColorBar()));

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

PineTimeStyle::~PineTimeStyle() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void PineTimeStyle::Refresh() {
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    if (batteryController.IsCharging()) {
      auto isCharging = batteryController.IsCharging() || batteryController.IsPowerPresent();
      lv_label_set_text(batteryPlug, BatteryIcon::GetPlugIcon(isCharging));
      lv_obj_realign(batteryPlug);
      lv_label_set_text(batteryIcon, "");
    } else {
      lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));
      lv_label_set_text(batteryPlug, "");
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
}
