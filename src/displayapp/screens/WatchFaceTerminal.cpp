#include <FreeRTOS.h>
#include <lvgl/lvgl.h>
#include "displayapp/screens/WatchFaceTerminal.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/WeatherHelper.h"
#include <nrfx_log.h>
#include <tuple>
#include <vector>
#include <cmath>

using namespace Pinetime::Applications::Screens;

WatchFaceTerminal::WatchFaceTerminal(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::HeartRateController& heartRateController,
                                     Controllers::MotionController& motionController,
                                     Controllers::SimpleWeatherService& weatherController
                                    )
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherController {weatherController} {
  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(batteryValue, true);
  lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -20);

  connectState = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(connectState, true);
  lv_obj_align(connectState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 80);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_LEFT_MID, 0, -100);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_date, true);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -40);

  label_prompt_1 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_prompt_1, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -80);
  lv_label_set_text_static(label_prompt_1, "user@watch:~ $ now");

  label_prompt_2 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_prompt_2, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 100);
  lv_label_set_text_static(label_prompt_2, "user@watch:~ $");

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_time, true);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -60);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(heartbeatValue, true);
  lv_obj_align(heartbeatValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 20);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(stepValue, true);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);
  
  weatherStatus = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(weatherStatus, true);
  lv_obj_align(weatherStatus, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 40);

  bytesFree = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(bytesFree, true);
  lv_obj_align(bytesFree, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 60);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceTerminal::~WatchFaceTerminal() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}
//https://www.wpc.ncep.noaa.gov/html/contract.html
  const char* WeatherString(uint8_t weatherID) {
    switch (weatherID) {
      case 0: {
        return "CLR";
      }
      case 1: {
        return "SLGT CLD";
      }
      case 2: {
       return "CLD"; 
      }
      case 3: {
       return "EXTRM CLD";
      }
      case 4: {
       return "TSRA";
      }
      case 5: {
       return "RA";
      }
      case 6: {
       return "TSTM";
      }
      case 7: {
       return "SN";
      }
      case 8: {
       return "BR";
      }
      default: {
       return "UNKN";
    }
    }
  }

void WatchFaceTerminal::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    lv_label_set_text_fmt(batteryValue, "[BATT]#387b54 %d%%", batteryPercentRemaining.Get());
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValue, LV_LABEL_POS_LAST, " Charging");
    }
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    if (!bleRadioEnabled.Get()) {
      lv_label_set_text_static(connectState, "[STAT]#0082fc Disabled#");
    } else {
      if (bleState.Get()) {
        lv_label_set_text_static(connectState, "[STAT]#0082fc Connected#");
      } else {
        lv_label_set_text_static(connectState, "[STAT]#0082fc Disconnected#");
      }
    }
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (notificationState.Get()) {
      lv_label_set_text_static(notificationIcon, "You have mail.");
    } else {
      lv_label_set_text_static(notificationIcon, "");
    }
  }
  
  // Following along from the example given by Weather.cpp...
  
  // get an instance of the weather service? not exactly sure what this does...
  currentWeather = weatherController.Current();
  // check to see if we have valid weather data
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get(); // the actual weather data
    if (optCurrentWeather) {
        int16_t temp = optCurrentWeather->temperature; // current temperature
        uint8_t weatherId = static_cast<int>(optCurrentWeather->iconId);    // weather type
        NRF_LOG_INFO("Raw temp: %d", temp);
        NRF_LOG_INFO("Rounded temp: %d", WeatherHelper::RoundTemperature(temp));
        //testColor(); //testVal * 100
        auto color = WeatherHelper::floatToRgbHex(WeatherHelper::TemperatureColor(temp)); // call temperature color BEFORE unit conversion
        // unit conversion
        char tempUnit = 'C';
        if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
          temp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(temp);
          tempUnit = 'F';
        }
        NRF_LOG_INFO("Color hex: %s", color);
        lv_label_set_text_fmt(weatherStatus, "[WTHR]#%s %d#°%c %s", color, WeatherHelper::RoundTemperature(temp), tempUnit, WeatherString(weatherId));
        delete[] color;
    } else {
      lv_label_set_text_static(weatherStatus, "[WTHR]No Data");
    }
  }

  lv_label_set_text_fmt(bytesFree, "[MEM] %i B Free", xPortGetFreeHeapSize());
  

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
      lv_label_set_text_fmt(label_time, "[TIME]#11cc55 %02d:%02d:%02d %s#", hour, minute, second, ampmChar);
    } else {
      lv_label_set_text_fmt(label_time, "[TIME]#11cc55 %02d:%02d:%02d", hour, minute, second);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      lv_label_set_text_fmt(label_date, "[DATE]#007fff %04d-%02d-%02d#", short(year), char(month), char(day));
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_label_set_text_fmt(heartbeatValue, "[L_HR]#ee3311 %d bpm#", heartbeat.Get());
    } else {
      lv_label_set_text_static(heartbeatValue, "[L_HR]#ee3311 ---#");
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "[STEP]#ee3377 %lu steps#", stepCount.Get());
  }
  
}
