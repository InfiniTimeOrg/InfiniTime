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
 * PineTimeStyle watch face for Infinitime created by Kieran Cawthray
 * Based on WatchFaceDigital
 * Style/layout copied from TimeStyle for Pebble by Dan Tilden (github.com/tilden)
 */

#include "displayapp/screens/WatchFacePineTimeStyle.h"
#include <lvgl/lvgl.h>
#include <cstdio>
#include <displayapp/Colors.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/DisplayApp.h"
#include "components/ble/weather/WeatherService.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFacePineTimeStyle*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

WatchFacePineTimeStyle::WatchFacePineTimeStyle(Controllers::DateTime& dateTimeController,
                                               const Controllers::Battery& batteryController,
                                               const Controllers::Ble& bleController,
                                               Controllers::NotificationManager& notificationManager,
                                               Controllers::Settings& settingsController,
                                               Controllers::MotionController& motionController,
                                               Controllers::WeatherService& weatherService)
  : currentDateTime {{}},
    batteryIcon(false),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController},
    weatherService {weatherService} {

  // Create a 200px wide background rectangle
  timebar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorBG()));
  lv_obj_set_style_local_radius(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(timebar, 200, 240);
  lv_obj_align(timebar, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  // Display the time
  timeDD1 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &open_sans_light);
  lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_label_set_text_static(timeDD1, "00");
  lv_obj_align(timeDD1, timebar, LV_ALIGN_IN_TOP_MID, 5, 5);

  timeDD2 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &open_sans_light);
  lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_label_set_text_static(timeDD2, "00");
  lv_obj_align(timeDD2, timebar, LV_ALIGN_IN_BOTTOM_MID, 5, -5);

  timeAMPM = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_obj_set_style_local_text_line_space(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, -3);
  lv_label_set_text_static(timeAMPM, "");
  lv_obj_align(timeAMPM, timebar, LV_ALIGN_IN_BOTTOM_LEFT, 2, -20);

  // Create a 40px wide bar down the right side of the screen
  sidebar = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorBar()));
  lv_obj_set_style_local_radius(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(sidebar, 40, 240);
  lv_obj_align(sidebar, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  // Display icons
  batteryIcon.Create(sidebar);
  batteryIcon.SetColor(LV_COLOR_BLACK);
  lv_obj_align(batteryIcon.GetObject(), nullptr, LV_ALIGN_IN_TOP_MID, 10, 2);

  plugIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(plugIcon, Symbols::plug);
  lv_obj_set_style_local_text_color(plugIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_align(plugIcon, sidebar, LV_ALIGN_IN_TOP_MID, 10, 2);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_align(bleIcon, sidebar, LV_ALIGN_IN_TOP_MID, -10, 2);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPTSColorTime()));
  lv_obj_align(notificationIcon, timebar, LV_ALIGN_IN_TOP_LEFT, 5, 5);

  weatherIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_text_font(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_label_set_text(weatherIcon, Symbols::cloudSunRain);
  lv_obj_align(weatherIcon, sidebar, LV_ALIGN_IN_TOP_MID, 0, 35);
  lv_obj_set_auto_realign(weatherIcon, true);
  if (settingsController.GetPTSWeather() == Pinetime::Controllers::Settings::PTSWeather::On) {
    lv_obj_set_hidden(weatherIcon, false);
  } else {
    lv_obj_set_hidden(weatherIcon, true);
  }

  temperature = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_align(temperature, sidebar, LV_ALIGN_IN_TOP_MID, 0, 65);
  if (settingsController.GetPTSWeather() == Pinetime::Controllers::Settings::PTSWeather::On) {
    lv_obj_set_hidden(temperature, false);
  } else {
    lv_obj_set_hidden(temperature, true);
  }

  // Calendar icon
  calendarOuter = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(calendarOuter, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_radius(calendarOuter, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(calendarOuter, 34, 34);
  if (settingsController.GetPTSWeather() == Pinetime::Controllers::Settings::PTSWeather::On) {
    lv_obj_align(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 20);
  } else {
    lv_obj_align(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 0);
  }

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
  lv_label_set_text_static(dateDayOfWeek, "THU");
  lv_obj_align(dateDayOfWeek, calendarOuter, LV_ALIGN_CENTER, 0, -32);

  dateDay = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateDay, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text_static(dateDay, "25");
  lv_obj_align(dateDay, calendarOuter, LV_ALIGN_CENTER, 0, 3);

  dateMonth = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(dateMonth, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text_static(dateMonth, "MAR");
  lv_obj_align(dateMonth, calendarOuter, LV_ALIGN_CENTER, 0, 32);

  // Step count gauge
  if (settingsController.GetPTSColorBar() == Pinetime::Controllers::Settings::Colors::White) {
    needle_colors[0] = LV_COLOR_BLACK;
  } else {
    needle_colors[0] = LV_COLOR_WHITE;
  }
  stepGauge = lv_gauge_create(lv_scr_act(), nullptr);
  lv_gauge_set_needle_count(stepGauge, 1, needle_colors);
  lv_gauge_set_range(stepGauge, 0, 100);
  lv_gauge_set_value(stepGauge, 0, 0);
  if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Full) {
    lv_obj_set_size(stepGauge, 40, 40);
    lv_obj_align(stepGauge, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_gauge_set_scale(stepGauge, 360, 11, 0);
    lv_gauge_set_angle_offset(stepGauge, 180);
    lv_gauge_set_critical_value(stepGauge, 100);
  } else if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Half) {
    lv_obj_set_size(stepGauge, 37, 37);
    lv_obj_align(stepGauge, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    lv_gauge_set_scale(stepGauge, 180, 5, 0);
    lv_gauge_set_angle_offset(stepGauge, 0);
    lv_gauge_set_critical_value(stepGauge, 120);
  } else if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Numeric) {
    lv_obj_set_hidden(stepGauge, true);
  }

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

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Numeric) {
    lv_obj_set_hidden(stepValue, false);
  } else {
    lv_obj_set_hidden(stepValue, true);
  }

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_TOP_MID, 0, 0);
  if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Numeric) {
    lv_obj_set_hidden(stepIcon, false);
  } else {
    lv_obj_set_hidden(stepIcon, true);
  }

  // Display seconds
  timeDD3 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(timeDD3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text_static(timeDD3, ":00");
  lv_obj_align(timeDD3, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  if (settingsController.GetPTSGaugeStyle() == Pinetime::Controllers::Settings::PTSGaugeStyle::Half) {
    lv_obj_set_hidden(timeDD3, false);
  } else {
    lv_obj_set_hidden(timeDD3, true);
  }

  btnNextTime = lv_btn_create(lv_scr_act(), nullptr);
  btnNextTime->user_data = this;
  lv_obj_set_size(btnNextTime, 60, 60);
  lv_obj_align(btnNextTime, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, -80);
  lv_obj_set_style_local_bg_opa(btnNextTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblNextTime = lv_label_create(btnNextTime, nullptr);
  lv_label_set_text_static(lblNextTime, ">");
  lv_obj_set_event_cb(btnNextTime, event_handler);
  lv_obj_set_hidden(btnNextTime, true);

  btnPrevTime = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevTime->user_data = this;
  lv_obj_set_size(btnPrevTime, 60, 60);
  lv_obj_align(btnPrevTime, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, -80);
  lv_obj_set_style_local_bg_opa(btnPrevTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblPrevTime = lv_label_create(btnPrevTime, nullptr);
  lv_label_set_text_static(lblPrevTime, "<");
  lv_obj_set_event_cb(btnPrevTime, event_handler);
  lv_obj_set_hidden(btnPrevTime, true);

  btnNextBar = lv_btn_create(lv_scr_act(), nullptr);
  btnNextBar->user_data = this;
  lv_obj_set_size(btnNextBar, 60, 60);
  lv_obj_align(btnNextBar, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 0);
  lv_obj_set_style_local_bg_opa(btnNextBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblNextBar = lv_label_create(btnNextBar, nullptr);
  lv_label_set_text_static(lblNextBar, ">");
  lv_obj_set_event_cb(btnNextBar, event_handler);
  lv_obj_set_hidden(btnNextBar, true);

  btnPrevBar = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevBar->user_data = this;
  lv_obj_set_size(btnPrevBar, 60, 60);
  lv_obj_align(btnPrevBar, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 0);
  lv_obj_set_style_local_bg_opa(btnPrevBar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblPrevBar = lv_label_create(btnPrevBar, nullptr);
  lv_label_set_text_static(lblPrevBar, "<");
  lv_obj_set_event_cb(btnPrevBar, event_handler);
  lv_obj_set_hidden(btnPrevBar, true);

  btnNextBG = lv_btn_create(lv_scr_act(), nullptr);
  btnNextBG->user_data = this;
  lv_obj_set_size(btnNextBG, 60, 60);
  lv_obj_align(btnNextBG, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 80);
  lv_obj_set_style_local_bg_opa(btnNextBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblNextBG = lv_label_create(btnNextBG, nullptr);
  lv_label_set_text_static(lblNextBG, ">");
  lv_obj_set_event_cb(btnNextBG, event_handler);
  lv_obj_set_hidden(btnNextBG, true);

  btnPrevBG = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevBG->user_data = this;
  lv_obj_set_size(btnPrevBG, 60, 60);
  lv_obj_align(btnPrevBG, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 80);
  lv_obj_set_style_local_bg_opa(btnPrevBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblPrevBG = lv_label_create(btnPrevBG, nullptr);
  lv_label_set_text_static(lblPrevBG, "<");
  lv_obj_set_event_cb(btnPrevBG, event_handler);
  lv_obj_set_hidden(btnPrevBG, true);

  btnReset = lv_btn_create(lv_scr_act(), nullptr);
  btnReset->user_data = this;
  lv_obj_set_size(btnReset, 60, 60);
  lv_obj_align(btnReset, lv_scr_act(), LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_local_bg_opa(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblReset = lv_label_create(btnReset, nullptr);
  lv_label_set_text_static(lblReset, "Rst");
  lv_obj_set_event_cb(btnReset, event_handler);
  lv_obj_set_hidden(btnReset, true);

  btnRandom = lv_btn_create(lv_scr_act(), nullptr);
  btnRandom->user_data = this;
  lv_obj_set_size(btnRandom, 60, 60);
  lv_obj_align(btnRandom, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(btnRandom, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblRandom = lv_label_create(btnRandom, nullptr);
  lv_label_set_text_static(lblRandom, "Rnd");
  lv_obj_set_event_cb(btnRandom, event_handler);
  lv_obj_set_hidden(btnRandom, true);

  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblClose = lv_label_create(btnClose, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  btnSteps = lv_btn_create(lv_scr_act(), nullptr);
  btnSteps->user_data = this;
  lv_obj_set_size(btnSteps, 160, 60);
  lv_obj_align(btnSteps, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);
  lv_obj_set_style_local_bg_opa(btnSteps, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblSteps = lv_label_create(btnSteps, nullptr);
  lv_label_set_text_static(lblSteps, "Steps style");
  lv_obj_set_event_cb(btnSteps, event_handler);
  lv_obj_set_hidden(btnSteps, true);

  btnWeather = lv_btn_create(lv_scr_act(), nullptr);
  btnWeather->user_data = this;
  lv_obj_set_size(btnWeather, 160, 60);
  lv_obj_align(btnWeather, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_local_bg_opa(btnWeather, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblWeather = lv_label_create(btnWeather, nullptr);
  lv_label_set_text_static(lblWeather, "Weather");
  lv_obj_set_event_cb(btnWeather, event_handler);
  lv_obj_set_hidden(btnWeather, true);

  btnSetColor = lv_btn_create(lv_scr_act(), nullptr);
  btnSetColor->user_data = this;
  lv_obj_set_size(btnSetColor, 150, 60);
  lv_obj_align(btnSetColor, lv_scr_act(), LV_ALIGN_CENTER, 0, -40);
  lv_obj_set_style_local_radius(btnSetColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_opa(btnSetColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_event_cb(btnSetColor, event_handler);
  lv_obj_t* lblSetColor = lv_label_create(btnSetColor, nullptr);
  lv_obj_set_style_local_text_font(lblSetColor, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(lblSetColor, Symbols::paintbrushLg);
  lv_obj_set_hidden(btnSetColor, true);

  btnSetOpts = lv_btn_create(lv_scr_act(), nullptr);
  btnSetOpts->user_data = this;
  lv_obj_set_size(btnSetOpts, 150, 60);
  lv_obj_align(btnSetOpts, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_local_radius(btnSetOpts, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_opa(btnSetOpts, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_event_cb(btnSetOpts, event_handler);
  lv_obj_t* lblSetOpts = lv_label_create(btnSetOpts, nullptr);
  lv_obj_set_style_local_text_font(lblSetOpts, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(lblSetOpts, Symbols::settings);
  lv_obj_set_hidden(btnSetOpts, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFacePineTimeStyle::~WatchFacePineTimeStyle() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

bool WatchFacePineTimeStyle::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_get_hidden(btnClose)) {
    lv_obj_set_hidden(btnSetColor, false);
    lv_obj_set_hidden(btnSetOpts, false);
    savedTick = lv_tick_get();
    return true;
  }
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && (lv_obj_get_hidden(btnClose) == false)) {
    return true;
  }
  return false;
}

void WatchFacePineTimeStyle::CloseMenu() {
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
  lv_obj_set_hidden(btnSteps, true);
  lv_obj_set_hidden(btnWeather, true);
}

bool WatchFacePineTimeStyle::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnClose)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFacePineTimeStyle::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}

void WatchFacePineTimeStyle::Refresh() {
  isCharging = batteryController.IsCharging();
  if (isCharging.IsUpdated()) {
    if (isCharging.Get()) {
      lv_obj_set_hidden(batteryIcon.GetObject(), true);
      lv_obj_set_hidden(plugIcon, false);
    } else {
      lv_obj_set_hidden(batteryIcon.GetObject(), false);
      lv_obj_set_hidden(plugIcon, true);
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
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
    lv_obj_realign(bleIcon);
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    auto hour = dateTimeController.Hours();
    auto minute = dateTimeController.Minutes();
    auto second = dateTimeController.Seconds();
    auto year = dateTimeController.Year();
    auto month = dateTimeController.Month();
    auto dayOfWeek = dateTimeController.DayOfWeek();
    auto day = dateTimeController.Day();

    if (displayedHour != hour || displayedMinute != minute) {
      displayedHour = hour;
      displayedMinute = minute;

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        char ampmChar[4] = "A\nM";
        if (hour == 0) {
          hour = 12;
        } else if (hour == 12) {
          ampmChar[0] = 'P';
        } else if (hour > 12) {
          hour = hour - 12;
          ampmChar[0] = 'P';
        }
        lv_label_set_text(timeAMPM, ampmChar);
        // Should be padded with blank spaces, but the space character doesn't exist in the font
        lv_label_set_text_fmt(timeDD1, "%02d", hour);
        lv_label_set_text_fmt(timeDD2, "%02d", minute);
      } else {
        lv_label_set_text_fmt(timeDD1, "%02d", hour);
        lv_label_set_text_fmt(timeDD2, "%02d", minute);
      }
    }

    if (displayedSecond != second) {
      displayedSecond = second;
      lv_label_set_text_fmt(timeDD3, ":%02d", second);
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_static(dateDayOfWeek, dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(dateDay, "%d", day);
      lv_obj_realign(dateDay);
      lv_label_set_text_static(dateMonth, dateTimeController.MonthShortToString());

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_gauge_set_value(stepGauge, 0, (stepCount.Get() / (settingsController.GetStepsGoal() / 100)) % 100);
    lv_obj_realign(stepGauge);
    lv_label_set_text_fmt(stepValue, "%luK", (stepCount.Get() / 1000));
    lv_obj_realign(stepValue);
    if (stepCount.Get() > settingsController.GetStepsGoal()) {
      lv_obj_set_style_local_line_color(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_scale_grad_color(stepGauge, LV_GAUGE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    }
  }

  if (weatherService.GetCurrentTemperature()->timestamp != 0 && weatherService.GetCurrentClouds()->timestamp != 0 &&
      weatherService.GetCurrentPrecipitation()->timestamp != 0) {
    nowTemp = (weatherService.GetCurrentTemperature()->temperature / 100);
    clouds = (weatherService.GetCurrentClouds()->amount);
    precip = (weatherService.GetCurrentPrecipitation()->amount);
    if (nowTemp.IsUpdated()) {
      lv_label_set_text_fmt(temperature, "%d°", nowTemp.Get());
      if ((clouds <= 30) && (precip == 0)) {
        lv_label_set_text(weatherIcon, Symbols::sun);
      } else if ((clouds >= 70) && (clouds <= 90) && (precip == 1)) {
        lv_label_set_text(weatherIcon, Symbols::cloudSunRain);
      } else if ((clouds > 90) && (precip == 0)) {
        lv_label_set_text(weatherIcon, Symbols::cloud);
      } else if ((clouds > 70) && (precip >= 2)) {
        lv_label_set_text(weatherIcon, Symbols::cloudShowersHeavy);
      } else {
        lv_label_set_text(weatherIcon, Symbols::cloudSun);
      };
      lv_obj_realign(temperature);
      lv_obj_realign(weatherIcon);
    }
  } else {
    lv_label_set_text_static(temperature, "--");
    lv_label_set_text(weatherIcon, Symbols::ban);
    lv_obj_realign(temperature);
    lv_obj_realign(weatherIcon);
  }

  if (!lv_obj_get_hidden(btnSetColor)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      lv_obj_set_hidden(btnSetColor, true);
      lv_obj_set_hidden(btnSetOpts, true);
      savedTick = 0;
    }
  }
}

void WatchFacePineTimeStyle::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  auto valueTime = settingsController.GetPTSColorTime();
  auto valueBar = settingsController.GetPTSColorBar();
  auto valueBG = settingsController.GetPTSColorBG();

  if (event == LV_EVENT_CLICKED) {
    if (object == btnNextTime) {
      valueTime = GetNext(valueTime);
      if (valueTime == valueBG) {
        valueTime = GetNext(valueTime);
      }
      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
    }
    if (object == btnPrevTime) {
      valueTime = GetPrevious(valueTime);
      if (valueTime == valueBG) {
        valueTime = GetPrevious(valueTime);
      }
      settingsController.SetPTSColorTime(valueTime);
      lv_obj_set_style_local_text_color(timeDD1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
    }
    if (object == btnNextBar) {
      valueBar = GetNext(valueBar);
      if (valueBar == Controllers::Settings::Colors::Black) {
        valueBar = GetNext(valueBar);
      }
      if (valueBar == Controllers::Settings::Colors::White) {
        needle_colors[0] = LV_COLOR_BLACK;
      } else {
        needle_colors[0] = LV_COLOR_WHITE;
      }
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBar));
    }
    if (object == btnPrevBar) {
      valueBar = GetPrevious(valueBar);
      if (valueBar == Controllers::Settings::Colors::Black) {
        valueBar = GetPrevious(valueBar);
      }
      if (valueBar == Controllers::Settings::Colors::White) {
        needle_colors[0] = LV_COLOR_BLACK;
      } else {
        needle_colors[0] = LV_COLOR_WHITE;
      }
      settingsController.SetPTSColorBar(valueBar);
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBar));
    }
    if (object == btnNextBG) {
      valueBG = GetNext(valueBG);
      if (valueBG == valueTime) {
        valueBG = GetNext(valueBG);
      }
      settingsController.SetPTSColorBG(valueBG);
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBG));
    }
    if (object == btnPrevBG) {
      valueBG = GetPrevious(valueBG);
      if (valueBG == valueTime) {
        valueBG = GetPrevious(valueBG);
      }
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
      lv_obj_set_style_local_text_color(timeDD2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(timeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      settingsController.SetPTSColorBar(static_cast<Controllers::Settings::Colors>(valueBar));
      lv_obj_set_style_local_bg_color(sidebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBar));
      settingsController.SetPTSColorBG(static_cast<Controllers::Settings::Colors>(valueBG));
      lv_obj_set_style_local_bg_color(timebar, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBG));
    }
    if (object == btnClose) {
      CloseMenu();
    }
    if (object == btnSteps) {
      if (!lv_obj_get_hidden(stepGauge) && (lv_obj_get_hidden(timeDD3))) {
        // show half gauge & seconds
        lv_obj_set_hidden(timeDD3, false);
        lv_obj_set_size(stepGauge, 37, 37);
        lv_obj_align(stepGauge, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
        lv_gauge_set_scale(stepGauge, 180, 5, 0);
        lv_gauge_set_angle_offset(stepGauge, 0);
        lv_gauge_set_critical_value(stepGauge, 120);
        settingsController.SetPTSGaugeStyle(Controllers::Settings::PTSGaugeStyle::Half);
      } else if (!lv_obj_get_hidden(timeDD3) && (lv_obj_get_hidden(stepValue))) {
        // show step count & icon
        lv_obj_set_hidden(timeDD3, true);
        lv_obj_set_hidden(stepGauge, true);
        lv_obj_set_hidden(stepValue, false);
        lv_obj_set_hidden(stepIcon, false);
        settingsController.SetPTSGaugeStyle(Controllers::Settings::PTSGaugeStyle::Numeric);
      } else {
        // show full gauge
        lv_obj_set_hidden(stepGauge, false);
        lv_obj_set_hidden(stepValue, true);
        lv_obj_set_hidden(stepIcon, true);
        lv_obj_set_size(stepGauge, 40, 40);
        lv_obj_align(stepGauge, sidebar, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
        lv_gauge_set_scale(stepGauge, 360, 11, 0);
        lv_gauge_set_angle_offset(stepGauge, 180);
        lv_gauge_set_critical_value(stepGauge, 100);
        settingsController.SetPTSGaugeStyle(Controllers::Settings::PTSGaugeStyle::Full);
      }
    }
    if (object == btnWeather) {
      if (lv_obj_get_hidden(weatherIcon)) {
        // show weather icon and temperature
        lv_obj_set_hidden(weatherIcon, false);
        lv_obj_set_hidden(temperature, false);
        lv_obj_align(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 20);
        lv_obj_realign(calendarInner);
        lv_obj_realign(calendarBar1);
        lv_obj_realign(calendarBar2);
        lv_obj_realign(calendarCrossBar1);
        lv_obj_realign(calendarCrossBar2);
        lv_obj_realign(dateDayOfWeek);
        lv_obj_realign(dateDay);
        lv_obj_realign(dateMonth);
        settingsController.SetPTSWeather(Controllers::Settings::PTSWeather::On);
      } else {
        // hide weather
        lv_obj_set_hidden(weatherIcon, true);
        lv_obj_set_hidden(temperature, true);
        lv_obj_align(calendarOuter, sidebar, LV_ALIGN_CENTER, 0, 0);
        lv_obj_realign(calendarInner);
        lv_obj_realign(calendarBar1);
        lv_obj_realign(calendarBar2);
        lv_obj_realign(calendarCrossBar1);
        lv_obj_realign(calendarCrossBar2);
        lv_obj_realign(dateDayOfWeek);
        lv_obj_realign(dateDay);
        lv_obj_realign(dateMonth);
        settingsController.SetPTSWeather(Controllers::Settings::PTSWeather::Off);
      }
    }
    if (object == btnSetColor) {
      lv_obj_set_hidden(btnSetColor, true);
      lv_obj_set_hidden(btnSetOpts, true);
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
    if (object == btnSetOpts) {
      lv_obj_set_hidden(btnSetColor, true);
      lv_obj_set_hidden(btnSetOpts, true);
      lv_obj_set_hidden(btnSteps, false);
      lv_obj_set_hidden(btnWeather, false);
      lv_obj_set_hidden(btnClose, false);
    }
  }
}

Pinetime::Controllers::Settings::Colors WatchFacePineTimeStyle::GetNext(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors nextColor;
  if (colorAsInt < 17) {
    nextColor = static_cast<Controllers::Settings::Colors>(colorAsInt + 1);
  } else {
    nextColor = static_cast<Controllers::Settings::Colors>(0);
  }
  return nextColor;
}

Pinetime::Controllers::Settings::Colors WatchFacePineTimeStyle::GetPrevious(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors prevColor;

  if (colorAsInt > 0) {
    prevColor = static_cast<Controllers::Settings::Colors>(colorAsInt - 1);
  } else {
    prevColor = static_cast<Controllers::Settings::Colors>(17);
  }
  return prevColor;
}
