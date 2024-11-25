#include "displayapp/screens/WatchFaceCasioStyleG7710.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "components/ble/SimpleWeatherService.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFaceCasioStyleG7710*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

WatchFaceCasioStyleG7710::WatchFaceCasioStyleG7710(Controllers::DateTime& dateTimeController,
                                                   const Controllers::Battery& batteryController,
                                                   const Controllers::Ble& bleController,
                                                   Controllers::NotificationManager& notificatioManager,
                                                   Controllers::Settings& settingsController,
                                                   Controllers::HeartRateController& heartRateController,
                                                   Controllers::MotionController& motionController,
                                                   Controllers::FS& filesystem,
                                                   Controllers::SimpleWeatherService& weatherService)
  : currentDateTime {{}},
    batteryIcon(false),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService} {

  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/lv_font_dots_40.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_dot40 = lv_font_load("F:/fonts/lv_font_dots_40.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/7segments_40.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_segment40 = lv_font_load("F:/fonts/7segments_40.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/7segments_115.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_segment115 = lv_font_load("F:/fonts/7segments_115.bin");
  }

  label_battery_value = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_battery_value, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_local_text_color(label_battery_value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(label_battery_value, "00%");

  batteryIcon.Create(lv_scr_act());
  batteryIcon.SetColor(color_text);
  lv_obj_align(batteryIcon.GetObject(), label_battery_value, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryPlug, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryIcon.GetObject(), LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, bleIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  label_day_of_week = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_day_of_week, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, 64);
  lv_obj_set_style_local_text_color(label_day_of_week, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_day_of_week, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_dot40);
  lv_label_set_text_static(label_day_of_week, "SUN");

  label_week_number = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_week_number, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 22);
  lv_obj_set_style_local_text_color(label_week_number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_week_number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_dot40);
  lv_label_set_text_static(label_week_number, "WK26");

  label_day_of_year = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_day_of_year, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 100, 30);
  lv_obj_set_style_local_text_color(label_day_of_year, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_day_of_year, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment40);
  lv_label_set_text_static(label_day_of_year, "181-184");

  weatherIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_label_set_text_static(weatherIcon, "");

  lv_obj_set_auto_realign(weatherIcon, true);
  if (settingsController.GetCasioWeather() == Pinetime::Controllers::Settings::PTSWeather::On) {
    lv_obj_set_hidden(weatherIcon, false);
  } else {
    lv_obj_set_hidden(weatherIcon, true);
  }

  label_temperature = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text(label_temperature, "");

  label_temperature_segment = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_temperature_segment, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_temperature_segment, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment40);
  lv_label_set_text(label_temperature_segment, "");

  lv_obj_set_hidden(label_temperature_segment, true);
  lv_obj_set_hidden(label_temperature, true);

  if (settingsController.GetCasioWeather() == Pinetime::Controllers::Settings::PTSWeather::On) {
    UpdateWeatherPosition();
  }

  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, LV_STATE_DEFAULT, 2);
  lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, color_text);
  lv_style_set_line_rounded(&style_line, LV_STATE_DEFAULT, true);

  lv_style_init(&style_border);
  lv_style_set_line_width(&style_border, LV_STATE_DEFAULT, 6);
  lv_style_set_line_color(&style_border, LV_STATE_DEFAULT, color_text);
  lv_style_set_line_rounded(&style_border, LV_STATE_DEFAULT, true);

  line_icons = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_icons, line_icons_points, 3);
  lv_obj_add_style(line_icons, LV_LINE_PART_MAIN, &style_line);
  lv_obj_align(line_icons, nullptr, LV_ALIGN_IN_TOP_RIGHT, -10, 18);

  line_day_of_week_number = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_day_of_week_number, line_day_of_week_number_points, 4);
  lv_obj_add_style(line_day_of_week_number, LV_LINE_PART_MAIN, &style_border);
  lv_obj_align(line_day_of_week_number, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 8);

  line_day_of_year = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_day_of_year, line_day_of_year_points, 3);
  lv_obj_add_style(line_day_of_year, LV_LINE_PART_MAIN, &style_line);
  lv_obj_align(line_day_of_year, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 60);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -45, 70);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment40);
  lv_label_set_text_static(label_date, "6-30");

  line_date = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_date, line_date_points, 3);
  lv_obj_add_style(line_date, LV_LINE_PART_MAIN, &style_line);
  lv_obj_align(line_date, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 100);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment115);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);

  line_time = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(line_time, line_time_points, 3);
  lv_obj_add_style(line_time, LV_LINE_PART_MAIN, &style_line);
  lv_obj_align(line_time, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, -25);

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_time_ampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 5, -5);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -2);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -2);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  btnWeather = lv_btn_create(lv_scr_act(), nullptr);
  btnWeather->user_data = this;
  lv_obj_set_size(btnWeather, 160, 60);
  lv_obj_align(btnWeather, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(btnWeather, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblWeather = lv_label_create(btnWeather, nullptr);
  lv_label_set_text_static(lblWeather, "Weather");
  lv_obj_set_event_cb(btnWeather, event_handler);
  lv_obj_set_hidden(btnWeather, true);

  btnSegment = lv_btn_create(lv_scr_act(), nullptr);
  btnSegment->user_data = this;
  lv_obj_set_size(btnSegment, 160, 60);
  lv_obj_align(btnSegment, lv_scr_act(), LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_local_bg_opa(btnSegment, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblSegment = lv_label_create(btnSegment, nullptr);
  lv_label_set_text_static(lblSegment, "Segment");
  lv_obj_set_event_cb(btnSegment, event_handler);
  lv_obj_set_hidden(btnSegment, true);

  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblClose = lv_label_create(btnClose, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceCasioStyleG7710::~WatchFaceCasioStyleG7710() {
  lv_task_del(taskRefresh);

  lv_style_reset(&style_line);
  lv_style_reset(&style_border);

  if (font_dot40 != nullptr) {
    lv_font_free(font_dot40);
  }

  if (font_segment40 != nullptr) {
    lv_font_free(font_segment40);
  }

  if (font_segment115 != nullptr) {
    lv_font_free(font_segment115);
  }

  lv_obj_clean(lv_scr_act());
}

void WatchFaceCasioStyleG7710::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_label_set_text_static(batteryPlug, BatteryIcon::GetPlugIcon(powerPresent.Get()));
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);
    lv_label_set_text_fmt(label_battery_value, "%d%%", batteryPercent);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }
  lv_obj_realign(label_battery_value);
  lv_obj_realign(batteryIcon.GetObject());
  lv_obj_realign(batteryPlug);
  lv_obj_realign(bleIcon);
  lv_obj_realign(notificationIcon);

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

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
      lv_label_set_text(label_time_ampm, ampmChar);
      lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
    }
    lv_obj_realign(label_time);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      const char* weekNumberFormat = "%V";

      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      int dayOfYear = dateTimeController.DayOfYear();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        // 24h mode: ddmmyyyy, first DOW=Monday;
        lv_label_set_text_fmt(label_date, "%3d-%2d", day, month);
        weekNumberFormat = "%V"; // Replaced by the week number of the year (Monday as the first day of the week) as a decimal number
                                 // [01,53]. If the week containing 1 January has four or more days in the new year, then it is considered
                                 // week 1. Otherwise, it is the last week of the previous year, and the next week is week 1. Both January
                                 // 4th and the first Thursday of January are always in week 1. [ tm_year, tm_wday, tm_yday]
      } else {
        // 12h mode: mmddyyyy, first DOW=Sunday;
        lv_label_set_text_fmt(label_date, "%3d-%2d", month, day);
        weekNumberFormat = "%U"; // Replaced by the week number of the year as a decimal number [00,53]. The first Sunday of January is the
                                 // first day of week 1; days in the new year before this are in week 0. [ tm_year, tm_wday, tm_yday]
      }

      time_t ttTime =
        std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(currentDateTime.Get()));
      tm* tmTime = std::localtime(&ttTime);

      // TODO: When we start using C++20, use std::chrono::year::is_leap
      int daysInCurrentYear = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0 ? 366 : 365;
      uint16_t daysTillEndOfYearNumber = daysInCurrentYear - dayOfYear;

      char buffer[8];
      strftime(buffer, 8, weekNumberFormat, tmTime);
      uint8_t weekNumber = atoi(buffer);

      lv_label_set_text_fmt(label_day_of_week, "%s", dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(label_day_of_year, "%3d-%3d", dayOfYear, daysTillEndOfYearNumber);
      lv_label_set_text_fmt(label_week_number, "WK%02d", weekNumber);

      lv_obj_realign(label_day_of_week);
      lv_obj_realign(label_day_of_year);
      lv_obj_realign(label_week_number);
      lv_obj_realign(label_date);
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color_text);
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepIcon);
  }

  if (settingsController.GetCasioWeather() == Pinetime::Controllers::Settings::PTSWeather::On) {
    DrawWeather();
  }

  if (!lv_obj_get_hidden(btnClose)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      CloseMenu();
      savedTick = 0;
    }
  }
}

void WatchFaceCasioStyleG7710::UpdateWeatherPosition() {
  lv_obj_set_hidden(label_day_of_year, false);
  lv_obj_set_hidden(label_week_number, false);
  lv_obj_set_hidden(label_day_of_week, false);

  lv_obj_set_hidden(weatherIcon, true);
  lv_obj_set_hidden(label_temperature_segment, true);
  lv_obj_set_hidden(label_temperature, true);

  if (settingsController.GetCasioWeather() == Controllers::Settings::PTSWeather::Off) {
    return;
  }

  lv_obj_set_hidden(weatherIcon, false);

  switch (settingsController.GetCasioWeatherSegment()) {
    case Controllers::Settings::CasioWeatherSegment::DayCounter:
      lv_obj_set_hidden(label_day_of_year, true);

      lv_obj_set_hidden(label_temperature_segment, false);
      lv_obj_align(weatherIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 110, 30);
      lv_obj_align(label_temperature_segment, nullptr, LV_ALIGN_IN_TOP_RIGHT, -5, 30);
      break;
    case Controllers::Settings::CasioWeatherSegment::WeekNumber:
      lv_obj_set_hidden(label_week_number, true);

      lv_obj_set_hidden(label_temperature, false);
      lv_obj_align(weatherIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 35, 15);
      lv_label_set_align(label_temperature, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(label_temperature, nullptr, LV_ALIGN_IN_TOP_LEFT, 20, 40);
      break;
    case Controllers::Settings::CasioWeatherSegment::DayOfWeek:
      lv_obj_set_hidden(label_day_of_week, true);

      lv_obj_set_hidden(label_temperature, false);
      lv_obj_align(weatherIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 35, 55);
      lv_label_set_align(label_temperature, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(label_temperature, nullptr, LV_ALIGN_IN_TOP_LEFT, 20, 80);
      break;
  }

  DrawWeather();
}

void WatchFaceCasioStyleG7710::DrawWeather() {
  currentWeather = weatherService.Current();

  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();

    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature.Celsius();
      char tempUnit = 'C';

      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = optCurrentWeather->temperature.Fahrenheit();
        tempUnit = 'F';
      }

      lv_label_set_text_fmt(label_temperature, "%d°%c", temp, tempUnit);
      lv_label_set_text_fmt(label_temperature_segment, "%d°%c", temp, tempUnit);
      lv_label_set_text(weatherIcon, Symbols::GetSymbol(optCurrentWeather->iconId));
    } else {
      lv_label_set_text_static(label_temperature, "");
      lv_label_set_text_static(label_temperature_segment, "");
      lv_label_set_text(weatherIcon, "");
    }

    lv_obj_realign(label_temperature);
    lv_obj_realign(label_temperature_segment);
    lv_obj_realign(weatherIcon);
  }
}

bool WatchFaceCasioStyleG7710::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/lv_font_dots_40.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/7segments_40.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/7segments_115.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  return true;
}

bool WatchFaceCasioStyleG7710::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (lv_obj_get_hidden(btnClose) == false) {
    savedTick = lv_tick_get();
  }
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_get_hidden(btnClose)) {
    lv_obj_set_hidden(btnWeather, false);
    lv_obj_set_hidden(btnSegment, false);

    lv_obj_set_hidden(btnClose, false);
    savedTick = lv_tick_get();
    return true;
  }
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && (lv_obj_get_hidden(btnClose) == false)) {
    return true;
  }
  return false;
}

void WatchFaceCasioStyleG7710::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_set_hidden(btnWeather, true);
  lv_obj_set_hidden(btnSegment, true);

  lv_obj_set_hidden(btnClose, true);
}

bool WatchFaceCasioStyleG7710::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnClose)) {
    CloseMenu();
    return true;
  }

  return false;
}

void WatchFaceCasioStyleG7710::HandleWeatherButton() {
  if (lv_obj_get_hidden(weatherIcon)) {
    settingsController.SetCasioWeather(Controllers::Settings::PTSWeather::On);
  } else {
    settingsController.SetCasioWeather(Controllers::Settings::PTSWeather::Off);
  }

  UpdateWeatherPosition();
}

void WatchFaceCasioStyleG7710::HandleSegmentButton() {
  switch (settingsController.GetCasioWeatherSegment()) {
    case Controllers::Settings::CasioWeatherSegment::DayCounter:
      settingsController.SetCasioWeatherSegment(Controllers::Settings::CasioWeatherSegment::WeekNumber);
      break;
    case Controllers::Settings::CasioWeatherSegment::WeekNumber:
      settingsController.SetCasioWeatherSegment(Controllers::Settings::CasioWeatherSegment::DayOfWeek);
      break;
    case Controllers::Settings::CasioWeatherSegment::DayOfWeek:
      settingsController.SetCasioWeatherSegment(Controllers::Settings::CasioWeatherSegment::DayCounter);
      break;
  }

  UpdateWeatherPosition();
}

void WatchFaceCasioStyleG7710::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (object == btnWeather) {
      HandleWeatherButton();
    }

    if (object == btnSegment) {
      HandleSegmentButton();
    }

    if (object == btnClose) {
      CloseMenu();
    }
  }
}
