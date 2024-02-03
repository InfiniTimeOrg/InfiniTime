#include <lvgl/lvgl.h>
#include "displayapp/screens/WatchFaceTerminal.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "displayapp/InfiniTimeTheme.h"


using namespace Pinetime::Applications::Screens;

namespace {
  lv_color_t temperatureColor(int16_t temperature) {
    if (temperature <= 0) { // freezing
      return Colors::blue;
    } else if (temperature <= 400) { // ice
      return LV_COLOR_CYAN;
    } else if (temperature >= 2700) { // hot
      return Colors::deepOrange;
    }
    return Colors::orange; // normal
  }
}

WatchFaceTerminal::WatchFaceTerminal(Controllers::DateTime& dateTimeController,
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

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_LEFT_MID, 0, -100);

  label_prompt_1 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_prompt_1, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -80);
  lv_label_set_text_static(label_prompt_1, "user@watch:~ $ now");

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::blue);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -60);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::blue);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -40);

  weather = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(weather, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -20);

  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 20);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(heartbeatValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 40);

  connectState = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(connectState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 60);

  label_prompt_2 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_prompt_2, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 80);
  lv_label_set_text_static(label_prompt_2, "user@watch:~ $");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceTerminal::~WatchFaceTerminal() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceTerminal::Refresh() {
  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (notificationState.Get()) {
      lv_label_set_text_static(notificationIcon, "You have mail.");
    } else {
      lv_label_set_text_static(notificationIcon, "");
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
      lv_label_set_text_fmt(label_time, "[TIME] %02d:%02d:%02d %s#", hour, minute, second, ampmChar);
    } else {
      lv_label_set_text_fmt(label_time, "[TIME] %02d:%02d:%02d", hour, minute, second);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      lv_label_set_text_fmt(label_date, "[DATE] %04d-%02d-%02d#", short(year), char(month), char(day));
    }
  }
  

  currentWeather = weatherService.Current();
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature;
      char tempUnit = 'C';
      lv_obj_set_style_local_text_color(weather, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, temperatureColor(temp));
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        condition = Symbols::GetSimpleCondition(optCurrentWeather->iconId);
        temp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(temp);
        tempUnit = 'F';
      }
      lv_label_set_text_fmt(weather, "[WTHR] %i°%c %s ", temp/100, tempUnit, condition);
    } else {
      lv_label_set_text(weather, "[WTHR] ---°");
      lv_obj_set_style_local_text_color(weather, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::gray);
    }
  }


  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    // HSV color model has red at 0° and green at 120°.
    // We lock satuation and brightness at 100% and traverse the cilinder
    // between red and green, thus avoiding the darker RGB on medium battery
    // charges and giving us a much nicer color range.
    uint8_t hue = batteryPercentRemaining.Get() * 120 / 100;
    lv_obj_set_style_local_text_color(batteryValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(hue, 100, 100));
    lv_label_set_text_fmt(batteryValue, "[BATT] %d%%", batteryPercentRemaining.Get());
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValue, LV_LABEL_POS_LAST, " Charging");
    }
  }


  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "[STEP] %lu steps#", stepCount.Get());
  }


  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {

      lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::deepOrange);
      lv_label_set_text_fmt(heartbeatValue, "[L_HR] %d bpm#", heartbeat.Get());
    } else {
      lv_label_set_text_static(heartbeatValue, "[L_HR] ---#");
      lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::gray);
    }
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    if (!bleRadioEnabled.Get()) {
      lv_label_set_text_static(connectState, "[STAT] Disabled#");
      lv_obj_set_style_local_text_color(connectState, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::gray);
    } else {
      if (bleState.Get()) {
        lv_label_set_text_static(connectState, "[STAT] Connected#");
        lv_obj_set_style_local_text_color(connectState, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::blue);
      } else {
        lv_label_set_text_static(connectState, "[STAT] Disconnected#");
        lv_obj_set_style_local_text_color(connectState, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::gray);
      }
    }
  }
}
