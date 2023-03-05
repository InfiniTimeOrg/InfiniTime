/*  Copyright (C) 2021 Avamander

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "Weather.h"
#include <lvgl/lvgl.h>
#include <components/ble/weather/WeatherService.h>
#include "Label.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/weather/WeatherData.h"

using namespace Pinetime::Applications::Screens;

Weather::Weather(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::WeatherService& weather)
  : app {app},
    weatherService(weather),
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return CreateScreenTemperature();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreenAir();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreenClouds();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreenPrecipitation();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreenHumidity();
              }},
             Screens::ScreenListModes::UpDown} {
}

Weather::~Weather() {
  lv_obj_clean(lv_scr_act());
}

void Weather::Refresh() {
  if (running) {
    // screens.Refresh();
  }
}

bool Weather::OnButtonPushed() {
  running = false;
  return true;
}

bool Weather::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

std::unique_ptr<Screen> Weather::CreateScreenTemperature() {
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  std::unique_ptr<Controllers::WeatherData::Temperature>& current = weatherService.GetCurrentTemperature();
  if (current->timestamp == 0) {
    // Do not use the data, it's invalid
    lv_label_set_text_fmt(label,
                          "#FFFF00 Temperature#\n\n"
                          "#444444 %d#°C \n\n"
                          "#444444 %d#\n\n"
                          "%d\n"
                          "%d\n",
                          0,
                          0,
                          0,
                          0);
  } else {
    lv_label_set_text_fmt(label,
                          "#FFFF00 Temperature#\n\n"
                          "#444444 %d#°C \n\n"
                          "#444444 %hd#\n\n"
                          "%llu\n"
                          "%lu\n",
                          current->temperature / 100,
                          current->dewPoint,
                          current->timestamp,
                          current->expires);
  }
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(0, 5, label));
}

std::unique_ptr<Screen> Weather::CreateScreenAir() {
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  std::unique_ptr<Controllers::WeatherData::AirQuality>& current = weatherService.GetCurrentQuality();
  if (current->timestamp == 0) {
    // Do not use the data, it's invalid
    lv_label_set_text_fmt(label,
                          "#FFFF00 Air quality#\n\n"
                          "#444444 %s#\n"
                          "#444444 %d#\n\n"
                          "%d\n"
                          "%d\n",
                          "",
                          0,
                          0,
                          0);
  } else {
    lv_label_set_text_fmt(label,
                          "#FFFF00 Air quality#\n\n"
                          "#444444 %s#\n"
                          "#444444 %lu#\n\n"
                          "%llu\n"
                          "%lu\n",
                          current->polluter.c_str(),
                          (current->amount / 100),
                          current->timestamp,
                          current->expires);
  }
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(0, 5, label));
}

std::unique_ptr<Screen> Weather::CreateScreenClouds() {
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  std::unique_ptr<Controllers::WeatherData::Clouds>& current = weatherService.GetCurrentClouds();
  if (current->timestamp == 0) {
    // Do not use the data, it's invalid
    lv_label_set_text_fmt(label,
                          "#FFFF00 Clouds#\n\n"
                          "#444444 %d%%#\n\n"
                          "%d\n"
                          "%d\n",
                          0,
                          0,
                          0);
  } else {
    lv_label_set_text_fmt(label,
                          "#FFFF00 Clouds#\n\n"
                          "#444444 %hhu%%#\n\n"
                          "%llu\n"
                          "%lu\n",
                          current->amount,
                          current->timestamp,
                          current->expires);
  }
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(0, 5, label));
}

std::unique_ptr<Screen> Weather::CreateScreenPrecipitation() {
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  std::unique_ptr<Controllers::WeatherData::Precipitation>& current = weatherService.GetCurrentPrecipitation();
  if (current->timestamp == 0) {
    // Do not use the data, it's invalid
    lv_label_set_text_fmt(label,
                          "#FFFF00 Precipitation#\n\n"
                          "#444444 %d%%#\n\n"
                          "%d\n"
                          "%d\n",
                          0,
                          0,
                          0);
  } else {
    lv_label_set_text_fmt(label,
                          "#FFFF00 Precipitation#\n\n"
                          "#444444 %hhu%%#\n\n"
                          "%llu\n"
                          "%lu\n",
                          current->amount,
                          current->timestamp,
                          current->expires);
  }
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(0, 5, label));
}

std::unique_ptr<Screen> Weather::CreateScreenHumidity() {
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  std::unique_ptr<Controllers::WeatherData::Humidity>& current = weatherService.GetCurrentHumidity();
  if (current->timestamp == 0) {
    // Do not use the data, it's invalid
    lv_label_set_text_fmt(label,
                          "#FFFF00 Humidity#\n\n"
                          "#444444 %d%%#\n\n"
                          "%d\n"
                          "%d\n",
                          0,
                          0,
                          0);
  } else {
    lv_label_set_text_fmt(label,
                          "#FFFF00 Humidity#\n\n"
                          "#444444 %hhu%%#\n\n"
                          "%llu\n"
                          "%lu\n",
                          current->humidity,
                          current->timestamp,
                          current->expires);
  }
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(0, 5, label));
}
