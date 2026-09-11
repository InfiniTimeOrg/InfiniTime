#include "displayapp/screens/WatchFacePrimeTime.h"

#include <lvgl/lvgl.h>
#include <cstdio>

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
#include "displayapp/InfiniTimeTheme.h"
#include "components/ble/MusicService.h"

using namespace Pinetime::Applications::Screens;

WatchFacePrimeTime::WatchFacePrimeTime(Controllers::DateTime& dateTimeController,
                                       const Controllers::Battery& batteryController,
                                       const Controllers::Ble& bleController,
                                       const Controllers::AlarmController& alarmController,
                                       Controllers::NotificationManager& notificationManager,
                                       Controllers::Settings& settingsController,
                                       Controllers::HeartRateController& heartRateController,
                                       Controllers::MotionController& motionController,
                                       Controllers::SimpleWeatherService& weatherService,
                                       Controllers::MusicService& music,
                                       Controllers::FS& filesystem)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService},
    musicService(music),
    statusIcons(batteryController, bleController, alarmController) {

  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/primetime.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    fontPrimeTime = lv_font_load("F:/fonts/primetime.bin");
  }

  statusIcons.Create();

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  weatherIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_text_font(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_label_set_text_static(weatherIcon, Symbols::ban);
  lv_obj_align(weatherIcon, nullptr, LV_ALIGN_IN_TOP_MID, 0, 30);
  lv_obj_set_auto_realign(weatherIcon, true);

  temperature = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text_static(temperature, "--°C");
  lv_obj_align(temperature, weatherIcon, LV_ALIGN_CENTER, 0, 25);

  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_CENTER, 0, 50);
  lv_obj_set_style_local_text_color(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  labelMusic = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(labelMusic, "%s Not Playing", Symbols::music);
  lv_obj_set_style_local_text_color(labelMusic, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_label_set_long_mode(labelMusic, LV_LABEL_LONG_SROLL_CIRC);
  lv_obj_set_width(labelMusic, LV_HOR_RES - 12);
  lv_label_set_align(labelMusic, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(labelMusic, lv_scr_act(), LV_ALIGN_CENTER, 0, 78);

  labelTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontPrimeTime);
  lv_obj_align(labelTime, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);

  labelTimeAmPm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelTimeAmPm, "");
  lv_obj_align(labelTimeAmPm, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -30, -55);

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 1);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xff4539));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0a84ff));
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0a84ff));
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFacePrimeTime::~WatchFacePrimeTime() {
  lv_task_del(taskRefresh);

  if (fontPrimeTime != nullptr) {
    lv_font_free(fontPrimeTime);
  }

  lv_obj_clean(lv_scr_act());
}

void WatchFacePrimeTime::Refresh() {
  statusIcons.Update();

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

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
      lv_label_set_text(labelTimeAmPm, ampmChar);
      lv_label_set_text_fmt(labelTime, "%2d:%02d", hour, minute);
      lv_obj_align(labelTime, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);
    } else {
      lv_label_set_text_fmt(labelTime, "%02d:%02d", hour, minute);
      lv_obj_align(labelTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      uint8_t day = dateTimeController.Day();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(labelDate,
                              "%s, %02d.%02d.%d",
                              dateTimeController.DayOfWeekShortToStringLow(dateTimeController.DayOfWeek()),
                              day,
                              dateTimeController.Month(),
                              year);
      } else {
        lv_label_set_text_fmt(labelDate,
                              "%s %s %d %d",
                              dateTimeController.DayOfWeekShortToString(),
                              dateTimeController.MonthShortToString(),
                              day,
                              year);
      }
      lv_obj_realign(labelDate);
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xff4539));
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
      lv_label_set_text_static(temperature, "--°");
      lv_label_set_text(weatherIcon, Symbols::ban);
    }
    lv_obj_realign(temperature);
    lv_obj_realign(weatherIcon);
  }
  if (track != musicService.getTrack()) {
    track = musicService.getTrack();
    lv_label_set_text_fmt(labelMusic, "%s %s", Symbols::music, track.data());
    lv_obj_realign(labelMusic);
  }
}

bool WatchFacePrimeTime::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/primetime.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  return true;
}
