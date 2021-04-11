#include "PineTimeStyle.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "BatteryIcon.h"
#include "BleIcon.h"
#include "NotificationIcon.h"
#include "Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/settings/Settings.h"
#include "../DisplayApp.h"

/*
 * PineTimeStyle watchface for Infinitime created by Kieran Cawthray
 * Based on WatchFaceDigital
 * Style/layout copied from TimeStyle for Pebble by Dan Tilden (github.com/tilden)
*/

using namespace Pinetime::Applications::Screens;


PineTimeStyle::PineTimeStyle(DisplayApp* app,
        Controllers::DateTime& dateTimeController,
        Controllers::Battery& batteryController,
        Controllers::Ble& bleController,
        Controllers::NotificationManager& notificatioManager,
        Controllers::Settings &settingsController,
        Controllers::HeartRateController& heartRateController): Screen(app), currentDateTime{{}},
                                           dateTimeController{dateTimeController}, batteryController{batteryController},
                                           bleController{bleController}, notificatioManager{notificatioManager},
                                           settingsController{settingsController},
                                           heartRateController{heartRateController} {
  /* This sets the watchface number to return to after leaving the menu*/
  settingsController.SetClockFace(2);

  displayedChar[0] = 0;
  displayedChar[1] = 0;
  displayedChar[2] = 0;
  displayedChar[3] = 0;
  displayedChar[4] = 0;

  /*Create a 200px wide background rectangle*/

  timebar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_set_size(timebar, 200, 240);
  lv_obj_align(timebar, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 0);

  /*Display the time*/
  timeDD1 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &open_sans_light);
  lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x008080));
  lv_label_set_text(timeDD1, "12");
  lv_obj_align(timeDD1, timebar, LV_ALIGN_IN_TOP_MID, 5, 5);

  timeDD2 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &open_sans_light);
  lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x008080));
  lv_label_set_text(timeDD2, "34");
  lv_obj_align(timeDD2, timebar, LV_ALIGN_IN_BOTTOM_MID, 5, -5);

  /*Create a 40px wide bar down the right side of the screen*/

  sidebar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x008080));
  lv_obj_set_size(sidebar, 40, 240);
  lv_obj_align(sidebar, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  /*Display icons*/
  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, sidebar, LV_ALIGN_IN_TOP_MID, 0, 2);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_IN_TOP_MID, 0, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, sidebar, LV_ALIGN_IN_TOP_LEFT, 7, 25);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, sidebar, LV_ALIGN_IN_TOP_RIGHT, -14, 25);

  /*Display date*/
  dateDayOfWeek = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(dateDayOfWeek, "THU");
  lv_obj_align(dateDayOfWeek, sidebar, LV_ALIGN_CENTER, 0, -20);

  dateDay = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(dateDay, "25");
  lv_obj_align(dateDay, sidebar, LV_ALIGN_CENTER, 0, 0);

  dateMonth = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(dateMonth, "MAR");
  lv_obj_align(dateMonth, sidebar, LV_ALIGN_CENTER, 0, 20);

  /*Display heartrate info*/
  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(heartbeatIcon, Symbols::heartBeat);
  lv_obj_align(heartbeatIcon, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, -50);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(heartbeatValue, "---");
  lv_obj_align(heartbeatValue, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, -30);

  heartbeatBpm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(heartbeatBpm, "BPM");
  lv_obj_align(heartbeatBpm, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, -5);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
}

PineTimeStyle::~PineTimeStyle() {
  lv_obj_clean(lv_scr_act());
}

bool PineTimeStyle::Refresh() {
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    if (batteryController.IsCharging()) {
      auto isCharging = batteryController.IsCharging() || batteryController.IsPowerPresent();
      lv_label_set_text(batteryPlug, BatteryIcon::GetPlugIcon(isCharging));
      lv_label_set_text(batteryIcon, "");
    } else {
      lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));
      lv_label_set_text(batteryPlug, "");
    }
  }


  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    if(bleState.Get() == true) {
      lv_label_set_text(bleIcon, BleIcon::GetIcon(true));
    } else {
      lv_label_set_text(bleIcon, BleIcon::GetIcon(false));
    }
  }

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if(notificationState.IsUpdated()) {
    if(notificationState.Get() == true)
      lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(true));
    else
      lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(false));
  }

  currentDateTime = dateTimeController.CurrentDateTime();

  if(currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime-dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto year = (int)yearMonthDay.year();
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>((unsigned)yearMonthDay.month());
    auto day = (unsigned)yearMonthDay.day();
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    int hour = time.hours().count();
    auto minute = time.minutes().count();

    char minutesChar[3];
    sprintf(minutesChar, "%02d", static_cast<int>(minute));

    char hoursChar[3];
    char ampmChar[3];
    if ( settingsController.GetClockType() == Controllers::Settings::ClockType::H24 ) {
      sprintf(hoursChar, "%02d", hour);
    } else {
      if (hour == 0 && hour != 12) {
          hour = 12;
          sprintf(ampmChar, "AM");
      }
      else if (hour == 12 && hour != 0) {
          hour = 12;
          sprintf(ampmChar, "PM");
      }
      else if (hour < 12 && hour != 0) {
          sprintf(ampmChar, "AM");
      }
      else if (hour > 12 && hour != 0)
      {
          hour = hour - 12;
          sprintf(ampmChar, "PM");
      }
      sprintf(hoursChar, "%02d", hour);
    }

    if(hoursChar[0] != displayedChar[0] || hoursChar[1] != displayedChar[1] || minutesChar[0] != displayedChar[2] || minutesChar[1] != displayedChar[3]) {
      displayedChar[0] = hoursChar[0];
      displayedChar[1] = hoursChar[1];
      displayedChar[2] = minutesChar[0];
      displayedChar[3] = minutesChar[1];

      char hourStr[3];
      char minStr[3];
      
      /*Display the time as 2 pairs of digits*/
      sprintf(hourStr, "%c%c", hoursChar[0],hoursChar[1]);
      lv_label_set_text(timeDD1, hourStr);
      
      sprintf(minStr, "%c%c", minutesChar[0], minutesChar[1]);
      lv_label_set_text(timeDD2, minStr);
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      char dayOfWeekStr[4];
      char dayStr[3];
      char monthStr[4];
      
      
      sprintf(dayOfWeekStr, "%s", dateTimeController.DayOfWeekShortToString());
      sprintf(dayStr, "%d", day);
      sprintf(monthStr, "%s", dateTimeController.MonthShortToString());
      
      lv_label_set_text(dateDayOfWeek, dayOfWeekStr);
      lv_label_set_text(dateDay, dayStr);
      lv_obj_align(dateDay, sidebar, LV_ALIGN_CENTER, 0, 0);
      lv_label_set_text(dateMonth, monthStr);
      
      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if(heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    char heartbeatBuffer[4];
    if(heartbeatRunning.Get())
      sprintf(heartbeatBuffer, "%d", heartbeat.Get());
    else
      sprintf(heartbeatBuffer, "---");

    lv_label_set_text(heartbeatValue, heartbeatBuffer);
    lv_obj_align(heartbeatValue, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, -30);
  }

  return running;
}