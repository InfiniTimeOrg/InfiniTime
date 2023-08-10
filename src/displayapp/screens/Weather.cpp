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
                return CreateTimelineStatsPage();
              },
              [this]() -> std::unique_ptr<Screen> {
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

std::unique_ptr<Screen> Weather::CreateTimelineStatsPage() {
  static constexpr uint8_t maxTimelineCount = 8;

  const auto& timeline = weatherService.GetEventTimeline();

  lv_obj_t* count_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(count_label, "Size: %u, Cap: %u", timeline.size(), timeline.capacity());

  lv_obj_t* timelineTable = lv_table_create(lv_scr_act(), nullptr);
  lv_obj_align(timelineTable, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 25);
  lv_table_set_col_cnt(timelineTable, 3);
  lv_table_set_row_cnt(timelineTable, maxTimelineCount + 1);
  lv_obj_set_style_local_pad_all(timelineTable, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_border_color(timelineTable, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xb0, 0xb0, 0xb0));

  lv_table_set_cell_value(timelineTable, 0, 0, "#");
  lv_table_set_col_width(timelineTable, 0, 30);
  lv_table_set_cell_value(timelineTable, 0, 1, "Type"); // State
  lv_table_set_col_width(timelineTable, 1, 50);
  lv_table_set_cell_value(timelineTable, 0, 2, "Expires");
  lv_table_set_col_width(timelineTable, 2, 150);

  for (uint8_t i = 0; i < timeline.size() && i < maxTimelineCount; i++) {
    char buffer[12] = {0};

    sprintf(buffer, "%u", i);
    lv_table_set_cell_value(timelineTable, i + 1, 0, buffer);

    auto& header = timeline[i];
    switch (header->eventType) {
      case Controllers::WeatherData::eventtype::Obscuration:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "Obsc");
        break;
      case Controllers::WeatherData::eventtype::Precipitation:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "Prec");
        break;
      case Controllers::WeatherData::eventtype::Wind:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "Wind");
        break;
      case Controllers::WeatherData::eventtype::Temperature:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "Temp");
        break;
      case Controllers::WeatherData::eventtype::AirQuality:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "AirQ");
        break;
      case Controllers::WeatherData::eventtype::Special:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "Spec");
        break;
      case Controllers::WeatherData::eventtype::Pressure:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "Press");
        break;
      case Controllers::WeatherData::eventtype::Location:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "Loc");
        break;
      case Controllers::WeatherData::eventtype::Clouds:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "Cloud");
        break;
      case Controllers::WeatherData::eventtype::Humidity:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "Humi");
        break;
      default:
        lv_table_set_cell_value(timelineTable, i + 1, 1, "????");
        break;
    }
    memset(buffer, 0, sizeof(buffer));
    uint64_t expired_at = header->timestamp + header->expires;
    sprintf(buffer, "%lu", static_cast<uint32_t>(expired_at));
    lv_table_set_cell_value(timelineTable, i + 1, 2, buffer);
  }

  return std::make_unique<Screens::Label>(3, 5, timelineTable);
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
