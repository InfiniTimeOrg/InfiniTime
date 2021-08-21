#include "Weather.h"
#include <lvgl/lvgl.h>
#include <components/ble/weather/WeatherService.h>
#include "../DisplayApp.h"
#include "Label.h"
#include "Version.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/brightness/BrightnessController.h"
#include "components/datetime/DateTimeController.h"
#include "drivers/Watchdog.h"
#include "components/ble/weather/WeatherData.h"

using namespace Pinetime::Applications::Screens;

Weather::Weather(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::WeatherService& weather)
  : Screen(app),
    dateTimeController {dateTimeController},
    weatherService(weather),
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return CreateScreenTemperature();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen2();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen3();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen4();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen5();
              }},
             Screens::ScreenListModes::UpDown} {
}

Weather::~Weather() {
  lv_obj_clean(lv_scr_act());
}

bool Weather::Refresh() {
  if (running) {
    screens.Refresh();
  }
  return running;
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
  Controllers::WeatherData::Temperature current = weatherService.GetCurrentTemperature();
  lv_label_set_text_fmt(label,
                        "#FFFF00 Temperature#\n\n"
                        "#444444 %hd%%#°C \n\n"
                        "#444444 %hd#\n"
                        "%llu\n"
                        "%lu\n",
                        current.temperature,
                        current.dewPoint,
                        current.timestamp,
                        current.expires);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(0, 5, app, label));
}

std::unique_ptr<Screen> Weather::CreateScreen2() {
  // uptime
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_fmt(label, "#444444 Date# %02d\n", dateTimeController.Day());
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(1, 4, app, label));
}

std::unique_ptr<Screen> Weather::CreateScreen3() {
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);

  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_fmt(label,
                        " #444444 frag# %d%%\n"
                        " #444444 free# %d",
                        mon.used_pct,
                        mon.frag_pct);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(2, 5, app, label));
}

bool sortById(const TaskStatus_t& lhs, const TaskStatus_t& rhs) {
  return lhs.xTaskNumber < rhs.xTaskNumber;
}

std::unique_ptr<Screen> Weather::CreateScreen4() {
  lv_obj_t* infoTask = lv_table_create(lv_scr_act(), nullptr);
  lv_table_set_col_cnt(infoTask, 3);
  lv_table_set_row_cnt(infoTask, 8);
  lv_obj_set_pos(infoTask, 10, 10);

  lv_table_set_cell_value(infoTask, 0, 0, "#");
  lv_table_set_col_width(infoTask, 0, 50);
  lv_table_set_cell_value(infoTask, 0, 1, "Task");
  lv_table_set_col_width(infoTask, 1, 80);
  lv_table_set_cell_value(infoTask, 0, 2, "Free");
  lv_table_set_col_width(infoTask, 2, 90);

  return std::unique_ptr<Screen>(new Screens::Label(3, 5, app, infoTask));
}

std::unique_ptr<Screen> Weather::CreateScreen5() {
  lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_static(label,
                           "Software Licensed\n"
                           "under the terms of\n"
                           "the GNU General\n"
                           "Public License v3\n"
                           "#444444 Source code#\n"
                           "#FFFF00 https://github.com/#\n"
                           "#FFFF00 JF002/InfiniTime#");
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  return std::unique_ptr<Screen>(new Screens::Label(4, 5, app, label));
}
