#include "displayapp/screens/WatchFaceGarden.h"

#include <lvgl/lvgl.h>

#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

WatchFaceGarden::WatchFaceGarden(Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 const Controllers::AlarmController& alarmController,
                                 Controllers::NotificationManager& notificationManager,
                                 Controllers::Settings& settingsController,
                                 Controllers::HeartRateController& heartRateController,
                                 Controllers::MotionController& motionController,
                                 Controllers::SimpleWeatherService& weatherService)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService},
    statusIcons(batteryController, bleController, alarmController) {

  statusIcons.Create();

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5FFF5F));
  lv_label_set_text_static(notificationIcon, "");
  lv_obj_align(notificationIcon, statusIcons.GetObject(), LV_ALIGN_OUT_LEFT_MID, -5, 0);

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF5F5F));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF5F5F));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5F8FFF));
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 70, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5F8FFF));
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, stepIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  garden = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(garden, 240, 217);
  lv_obj_set_style_local_radius(garden, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_bg_grad_dir(garden, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
  lv_obj_align(garden, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  label_time_colon = lv_label_create(garden, nullptr);
  lv_label_set_text(label_time_colon, ":");
  lv_obj_set_style_local_text_font(label_time_colon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &azeret_mono);
  lv_obj_align(label_time_colon, garden, LV_ALIGN_IN_TOP_MID, 0, 10);

  label_time_hour = lv_label_create(garden, nullptr);
  lv_obj_set_style_local_text_font(label_time_hour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &azeret_mono);
  lv_obj_align(label_time_hour, label_time_colon, LV_ALIGN_OUT_LEFT_MID, 12, 0);

  label_time_min = lv_label_create(garden, nullptr);
  lv_obj_set_style_local_text_font(label_time_min, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &azeret_mono);
  lv_obj_align(label_time_min, label_time_colon, LV_ALIGN_OUT_RIGHT_MID, -12, 0);

  label_time_ampm = lv_label_create(garden, nullptr);
  lv_label_set_text(label_time_ampm, "");
  lv_obj_align(label_time_ampm, label_time_colon, LV_ALIGN_IN_TOP_MID, 0, -7);

  label_date = lv_label_create(garden, nullptr);
  lv_obj_align(label_date, garden, LV_ALIGN_IN_TOP_LEFT, 4, 75);

  temperature = lv_label_create(garden, nullptr);
  lv_label_set_text(temperature, "");
  lv_obj_align(temperature, garden, LV_ALIGN_IN_TOP_RIGHT, -4, 75);

  weatherIcon = lv_label_create(garden, nullptr);
  lv_obj_set_style_local_text_font(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_label_set_text(weatherIcon, "");
  lv_obj_align(weatherIcon, temperature, LV_ALIGN_OUT_LEFT_MID, -3, 0);

  for (int k = 0; k < NUM_FLOWERS; k++) {
    flowers[k].Create(garden);
    lv_obj_align(flowers[k].GetObject(), garden, LV_ALIGN_IN_BOTTOM_MID, X_POS_OFFSET + (X_POS_STEP * k), 0);
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceGarden::~WatchFaceGarden() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceGarden::Refresh() {
  bool refreshGarden = false;
  bool realignNotification = statusIcons.Update();

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text(notificationIcon, (notificationState.Get() ? Symbols::lapsFlag : ""));
    realignNotification = true;
  }

  if (realignNotification) {
    lv_obj_realign(notificationIcon);
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    int offset = 0;

    if (hour < 6) {
      stage = 0;
    } else if (hour < 9) {
      stage = 1;
    } else if (hour < 11) {
      stage = 2;
    } else if (hour < 13) {
      stage = 3;
    } else if (hour < 20) {
      stage = 4;
    } else if (hour < 23) {
      stage = 5;
    } else {
      stage = 0;
    }

    if (stage.IsUpdated()) {
      lv_obj_set_style_local_bg_color(garden, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(BG_COLORS[stage.Get() * 2]));
      lv_obj_set_style_local_bg_grad_color(garden, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(BG_COLORS[stage.Get() * 2 + 1]));
      refreshGarden = true;
    }

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      lv_label_set_text(label_time_ampm, (hour >= 12 ? "PM" : "AM"));

      if (hour == 0) {
        hour = 12;
      } else if (hour > 12) {
        hour = hour - 12;
      }
      lv_label_set_text_fmt(label_time_hour, "%2d", hour);

      if (hour < 10) {
        offset = -24; // center time display
      }
    } else {
      lv_label_set_text_fmt(label_time_hour, "%02d", hour);
    }
    lv_obj_align(label_time_colon, garden, LV_ALIGN_IN_TOP_MID, offset, 7);
    lv_obj_realign(label_time_hour);
    lv_obj_realign(label_time_ampm);

    lv_label_set_text_fmt(label_time_min, "%02d", minute);
    lv_obj_realign(label_time_min);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint8_t day = dateTimeController.Day();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(label_date,
                              "%s %d %s",
                              dateTimeController.DayOfWeekShortToString(),
                              day,
                              dateTimeController.MonthShortToString());
      } else {
        lv_label_set_text_fmt(label_date,
                              "%s %s %d",
                              dateTimeController.DayOfWeekShortToString(),
                              dateTimeController.MonthShortToString(),
                              day);
      }
      lv_obj_realign(label_date);

      refreshGarden = true;
      // base seed is the date
      seeds[0] = ((uint8_t) dateTimeController.Month() * 32U) + day;
      for (int k = 1; k < NUM_FLOWERS; k++) {
        // "sow" seeds using PRNG (xorshift32)
        uint32_t s = seeds[k - 1];
        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        seeds[k] = s;
      }
    }

    if (refreshGarden) {
      for (int k = 0; k < NUM_FLOWERS; k++) {
        flowers[k].Update(seeds[k], stage.Get());
        lv_obj_realign(flowers[k].GetObject());
      }
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF5F5F));
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5F5F5F));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepIcon);
    lv_obj_realign(stepValue);
  }

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
      lv_label_set_text_fmt(temperature, "%d°%c", temp, tempUnit);
      lv_label_set_text(weatherIcon, Symbols::GetSymbol(optCurrentWeather->iconId));
    } else {
      lv_label_set_text_static(temperature, "");
      lv_label_set_text(weatherIcon, "");
    }
    lv_obj_realign(temperature);
    lv_obj_realign(weatherIcon);
  }
}
