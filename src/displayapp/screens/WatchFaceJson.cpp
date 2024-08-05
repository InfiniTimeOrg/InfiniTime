#include <lvgl/lvgl.h>
#include "displayapp/screens/WatchFaceJson.h"
#include "displayapp/screens/BatteryIcon.h"
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

#define PROP_OFFSET 12
#define LINE_HEIGHT 20

using namespace Pinetime::Applications::Screens;

WatchFaceJson::WatchFaceJson(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::HeartRateController& heartRateController,
                                     Controllers::MotionController& motionController,
                                     Controllers::SimpleWeatherService& weatherService)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService} {
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x30, 0x38, 0x45));

  notification_icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(notification_icon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);


  label_open = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_open, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_style_local_text_font(label_open, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_open, true);
  lv_label_set_text_static(label_open, "#A6B2C0 {#");

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, PROP_OFFSET, LINE_HEIGHT);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_time, true);
  lv_label_set_text_static(label_time, "#C792EA \"time\"##A6B2C0 :##92D69E \"12:34\"##A6B2C0 ,#");
  
  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, PROP_OFFSET, LINE_HEIGHT*2);
  lv_obj_set_style_local_text_font(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_date, true);
  lv_label_set_text_static(label_date, "#C792EA \"date\"##A6B2C0 :##92D69E \"1970-01-01\"##A6B2C0 ,#");

  label_battery_open = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_battery_open, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, PROP_OFFSET, LINE_HEIGHT*3);
  lv_obj_set_style_local_text_font(label_battery_open, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_battery_open, true);
  lv_label_set_text_static(label_battery_open, "#C792EA \"battery\"##A6B2C0 : {#");

  label_battery_percent = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_battery_percent, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, (PROP_OFFSET*2), LINE_HEIGHT*4);
  lv_obj_set_style_local_text_font(label_battery_percent, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_battery_percent, true);
  lv_label_set_text_static(label_battery_percent, "#C792EA \"percent\"##A6B2C0 :##D19A66 0.69##A6B2C0 ,#");

  label_battery_status = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_battery_status, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, (PROP_OFFSET*2), LINE_HEIGHT*5);
  lv_obj_set_style_local_text_font(label_battery_status, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_battery_status, true);
  lv_label_set_text_static(label_battery_status, "#C792EA \"state\"##A6B2C0 :##92D69E \"discharging\"#");

  label_battery_close = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_battery_close, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, PROP_OFFSET, LINE_HEIGHT*6);
  lv_obj_set_style_local_text_font(label_battery_close, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_battery_close, true);
  lv_label_set_text_static(label_battery_close, "#A6B2C0 },#");

  label_steps = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_steps, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, PROP_OFFSET, LINE_HEIGHT*7);
  lv_obj_set_style_local_text_font(label_steps, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_steps, true);
  lv_label_set_text_static(label_steps, "#C792EA \"steps\"##A6B2C0 :##D19A66 0##A6B2C0 ,#");

  label_status = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_status, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, PROP_OFFSET, LINE_HEIGHT*8);
  lv_obj_set_style_local_text_font(label_status, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_status, true);
  lv_label_set_text_static(label_status, "#C792EA \"ble\"##A6B2C0 :##92D69E \"connected\"##A6B2C0 ,#");

  label_weather = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_weather, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, PROP_OFFSET, LINE_HEIGHT*9);
  lv_obj_set_style_local_text_font(label_weather, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_weather, true);
  lv_label_set_text_static(label_weather, "#C792EA \"weather\"##A6B2C0 :##92D69E  \"   \"##A6B2C0 ,#");

  weather_icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(weather_icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_obj_set_style_local_text_color(weather_icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x92, 0xD6, 0x9E));
  lv_label_set_text(weather_icon, Symbols::ban);
  lv_obj_align(weather_icon, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 133, (LINE_HEIGHT*9)-3);
  
  lv_obj_set_auto_realign(weather_icon, true);

  label_temperature = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_temperature, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, PROP_OFFSET, LINE_HEIGHT*10);
  lv_obj_set_style_local_text_font(label_temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_temperature, true);
  lv_label_set_text_static(label_temperature, "#C792EA \"temperature\"##A6B2C0 :##D19A66 -20#");

  label_close = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_close, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, LINE_HEIGHT*11);
  lv_obj_set_style_local_text_font(label_close, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_16);
  lv_label_set_recolor(label_close, true);
  lv_label_set_text_static(label_close, "#A6B2C0 }#");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceJson::~WatchFaceJson() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceJson::Refresh() {
  currentWeather = weatherService.Current();

  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();

    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature;
      
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(temp);
      }

      temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);

      lv_label_set_text(weather_icon, Symbols::GetSymbol(optCurrentWeather->iconId));
      lv_label_set_text_fmt(label_temperature, "#C792EA \"temperature\"##A6B2C0 :##D19A66  %d#", temp);
    } else {
      lv_label_set_text(weather_icon, Symbols::ban);
      lv_label_set_text_static(label_temperature, "#C792EA \"temperature\"##A6B2C0 :##D19A66  null#");
    }

    lv_obj_realign(weather_icon);
  }

  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    lv_label_set_text_fmt(label_battery_percent, "#C792EA \"percent\"##A6B2C0 :##D19A66  %.2f##A6B2C0 ,#", (batteryPercentRemaining.Get()/100));
    if (batteryController.IsPowerPresent()) {
      lv_label_set_text_static(label_battery_status, "#C792EA \"state\"##A6B2C0 :##92D69E  \"charging\"#");
    } else {
      lv_label_set_text_static(label_battery_status, "#C792EA \"state\"##A6B2C0 :##92D69E  \"discharging\"#");
    }
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    if (!bleRadioEnabled.Get()) {
      lv_label_set_text_static(label_status, "#C792EA \"ble\"##A6B2C0 :##92D69E  \"disabled\"##A6B2C0 ,#");
    } else {
      if (bleState.Get()) {
        lv_label_set_text_static(label_status, "#C792EA \"ble\"##A6B2C0 :##92D69E  \"connected\"##A6B2C0 ,#");
      } else {
        lv_label_set_text_static(label_status, "#C792EA \"ble\"##A6B2C0 :##92D69E  \"disconnected\"##A6B2C0 ,#");
      }
    }
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (notificationState.Get()) {
      lv_label_set_text_static(notification_icon, NotificationIcon::GetIcon(notificationState.Get()));
    } else {
      lv_label_set_text_static(notification_icon, "");
    }
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    uint8_t second = dateTimeController.Seconds();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text_fmt(label_time, "#C792EA \"time\"##A6B2C0 :##92D69E  \"%02d:%02d:%02d %s\"##A6B2C0 ,#", hour, minute, second, ampmChar);
    } else {
      lv_label_set_text_fmt(label_time, "#C792EA \"time\"##A6B2C0 :##92D69E  \"%02d:%02d:%02d\"##A6B2C0 ,#", hour, minute, second);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      lv_label_set_text_fmt(label_date, "#C792EA \"date\"##A6B2C0 :##92D69E  \"%04d-%02d-%02d\"##A6B2C0 ,#", short(year), char(month), char(day));
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(label_steps, "#C792EA \"steps\"##A6B2C0 :##D19A66  %lu##A6B2C0 ,#", stepCount.Get());
  }
}
