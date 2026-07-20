#include "displayapp/screens/WatchFaceLegend.h"

#include <lvgl/lvgl.h>
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

using namespace Pinetime::Applications::Screens;

WatchFaceLegend::WatchFaceLegend(Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 const Controllers::AlarmController& alarmController,
                                 Controllers::Timer& timerController,
                                 Controllers::StopWatchController& stopWatchController,
                                 Controllers::NotificationManager& notificationManager,
                                 Controllers::Settings& settingsController,
                                 Controllers::HeartRateController& heartRateController,
                                 Controllers::MotionController& motionController,
                                 Controllers::SimpleWeatherService& weatherService)
  : dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    alarmController {alarmController},
    timerController {timerController},
    stopWatchController {stopWatchController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService} {

  // ---- Top status strip -------------------------------------------------
  // Any notification indicator (top left). Hidden when notifications are empty.
  notificationAnyDot = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(notificationAnyDot, 7, 7);
  lv_obj_set_style_local_bg_color(notificationAnyDot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, colorAnyNotification);
  lv_obj_set_style_local_radius(notificationAnyDot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(notificationAnyDot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_align(notificationAnyDot, nullptr, LV_ALIGN_IN_TOP_LEFT, 18, 16);
  lv_obj_set_hidden(notificationAnyDot, true); // hidden by default

  // Standard new notification indicator (used when GetNotificationStatus() is Off or Sleep).
  notificationNewIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationNewIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorRed);
  lv_label_set_text_static(notificationNewIcon, "");
  lv_obj_align(notificationNewIcon, notificationAnyDot, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  lv_obj_set_auto_realign(notificationNewIcon, true);

  // Services icons. Empty string when nothing is active, can contain multiple icons.
  servicesIcons = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(servicesIcons, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorSecondary);
  lv_label_set_text_static(servicesIcons, "");
  lv_obj_align(servicesIcons, notificationNewIcon, LV_ALIGN_OUT_RIGHT_MID, 12, 1);
  lv_obj_set_auto_realign(servicesIcons, true);

  // Numeric battery (top right).
  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorSecondary);
  lv_label_set_text_static(batteryValue, "--%");
  lv_obj_align(batteryValue, nullptr, LV_ALIGN_IN_TOP_RIGHT, -5, 10);
  lv_obj_set_auto_realign(batteryValue, true);

  // BLE icon, left of the battery percentage.
  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorBlue);
  lv_label_set_text_static(bleIcon, "");
  lv_obj_align(bleIcon, batteryValue, LV_ALIGN_OUT_LEFT_MID, -10, 0);
  lv_obj_set_auto_realign(bleIcon, true);

  // Upper divider - displayed (red or blue) when GetNotificationStatus() is Off or Sleep
  upperDivider = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(upperDivider, dividerPoints, 2);
  lv_obj_set_style_local_line_width(upperDivider, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_align(upperDivider, nullptr, LV_ALIGN_IN_TOP_LEFT, 4, 45);
  lv_obj_set_hidden(upperDivider, true); // hidden by default

  // ---- Center block ------------------------------------------------------
  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorSecondary);
  lv_label_set_text_static(labelDate, "");
  lv_obj_align(labelDate, nullptr, LV_ALIGN_IN_TOP_LEFT, 4, 64);

  labelTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
  lv_obj_set_style_local_text_color(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorForeground);
  lv_label_set_text_static(labelTime, "");
  lv_obj_align(labelTime, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 94);

  labelAmPm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelAmPm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorForeground);
  lv_label_set_text_static(labelAmPm, "");
  lv_obj_align(labelAmPm, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 64);
  lv_obj_set_auto_realign(labelAmPm, true);

  divider = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(divider, dividerPoints, 2);
  lv_obj_set_style_local_line_color(divider, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, colorDivider);
  lv_obj_set_style_local_line_width(divider, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_align(divider, nullptr, LV_ALIGN_IN_TOP_LEFT, 4, 177);

  // ---- Bottom row --------------------------------------------------
  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorSecondary);
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, nullptr, LV_ALIGN_IN_TOP_LEFT, 4, 186);

  stepCaption = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepCaption, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorCaption);
  lv_label_set_text_static(stepCaption, "steps");
  lv_obj_align(stepCaption, nullptr, LV_ALIGN_IN_TOP_LEFT, 4, 210);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorSecondary);
  lv_label_set_text_static(heartbeatValue, "--");
  lv_obj_align(heartbeatValue, nullptr, LV_ALIGN_IN_TOP_LEFT, 100, 186);

  heartbeatCaption = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatCaption, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorCaption);
  lv_label_set_text_static(heartbeatCaption, "BPM");
  lv_obj_align(heartbeatCaption, nullptr, LV_ALIGN_IN_TOP_LEFT, 100, 210);

  weatherValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(weatherValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorSecondary);
  lv_label_set_text_static(weatherValue, "--");
  lv_obj_align(weatherValue, nullptr, LV_ALIGN_IN_TOP_LEFT, 188, 186);

  weatherCaption = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(weatherCaption, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorCaption);
  lv_label_set_text_static(weatherCaption, "\xC2\xB0");
  lv_obj_align(weatherCaption, nullptr, LV_ALIGN_IN_TOP_LEFT, 188, 210);

  weatherIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, colorSecondary);
  lv_obj_set_style_local_text_font(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_label_set_text_static(weatherIcon, "");
  lv_obj_align(weatherIcon, weatherValue, LV_ALIGN_OUT_LEFT_BOTTOM, -5, 12);
  lv_obj_set_auto_realign(weatherIcon, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceLegend::~WatchFaceLegend() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceLegend::Refresh() {
  // ---- Notification indicators ----
  notificationState = notificationManager.AreNewNotificationsAvailable();
  notificationEmpty = notificationManager.IsEmpty();
  if (notificationState.IsUpdated() || notificationEmpty.IsUpdated()) {
    bool state = notificationState.Get();
    bool empty = notificationEmpty.Get();
    lv_obj_set_hidden(notificationAnyDot, empty && !state);
    lv_label_set_text_static(notificationNewIcon, NotificationIcon::GetIcon(state));
    lv_obj_align(servicesIcons, state ? notificationNewIcon : notificationAnyDot, LV_ALIGN_OUT_RIGHT_MID, empty && !state ? -18 : 12, 1);
  }

  // ---- Services (Alarm, Timer, StopWatch) ----
  alarmEnabled = alarmController.IsEnabled();
  timerRunning = timerController.IsRunning();
  stopWatchRunning = stopWatchController.IsRunning();
  if (alarmEnabled.IsUpdated() || timerRunning.IsUpdated() || stopWatchRunning.IsUpdated()) {
    lv_label_set_text_fmt(servicesIcons,
                          "%s%s%s%s%s",
                          stopWatchRunning.Get() ? Symbols::stopWatch : "",
                          stopWatchRunning.Get() ? " " : "",
                          alarmEnabled.Get() ? Symbols::bell : "",
                          alarmEnabled.Get() ? " " : "",
                          timerRunning.Get() ? Symbols::hourGlass : "");
  }

  // ---- BLE ----
  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }

  // ---- Battery ----
  batteryPercentRemaining = batteryController.PercentRemaining();
  powerPresent = batteryController.IsPowerPresent();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    uint8_t remaining = batteryPercentRemaining.Get();
    lv_label_set_text_fmt(batteryValue, "%d%%", remaining);
    lv_obj_set_style_local_text_color(batteryValue,
                                      LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT,
                                      powerPresent.Get() ? colorCharging
                                      : remaining > 20   ? colorSecondary
                                                         : colorRed);
    lv_obj_align(bleIcon, batteryValue, LV_ALIGN_OUT_LEFT_MID, -10, 0);
  }

  // ---- Upper divider, based on notification config ----
  notificationConfig = settingsController.GetNotificationStatus();
  if (notificationConfig.IsUpdated()) {
    switch (notificationConfig.Get()) {
      case Controllers::Settings::Notification::Off:
        lv_obj_set_style_local_line_color(upperDivider, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, colorRed);
        lv_obj_set_hidden(upperDivider, false);
        break;
      case Controllers::Settings::Notification::Sleep:
        lv_obj_set_style_local_line_color(upperDivider, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, colorBlue);
        lv_obj_set_hidden(upperDivider, false);
        break;
      default:
        lv_obj_set_hidden(upperDivider, true);
        break;
    }
  }

  // ---- Time and date ----
  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      const char* ampm = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampm = "PM";
      } else if (hour > 12) {
        hour = hour - 12;
        ampm = "PM";
      }
      lv_label_set_text_static(labelAmPm, ampm);
    } else {
      lv_label_set_text_static(labelAmPm, "");
    }
    lv_label_set_text_fmt(labelTime, "%02d:%02d", hour, minute);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      lv_label_set_text_fmt(labelDate,
                            "%s %02d %s",
                            dateTimeController.DayOfWeekShortToString(),
                            dateTimeController.Day(),
                            dateTimeController.MonthShortToString());
    }
  }

  // ---- Steps ----
  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
  }

  // ---- Heart rate ----
  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_label_set_text_static(heartbeatValue, "--");
    }
  }

  // ---- Weather ----
  currentWeather = weatherService.Current();
  if (currentWeather.IsUpdated()) {
    if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
      lv_label_set_text_static(weatherCaption,
                               "\xC2\xB0"
                               "F");
    } else {
      lv_label_set_text_static(weatherCaption,
                               "\xC2\xB0"
                               "C");
    }

    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      int16_t temp;
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = optCurrentWeather->temperature.Fahrenheit();
      } else {
        temp = optCurrentWeather->temperature.Celsius();
      }
      if (temp > -100) {
        lv_label_set_text_fmt(weatherValue, "%d", temp);
      } else {
        lv_label_set_text_static(weatherValue, "--");
      }

      lv_color_t color;
      switch (optCurrentWeather->iconId) {
        case Controllers::SimpleWeatherService::Icons::Sun:
          color = colorWeatherYellow;
          break;
        case Controllers::SimpleWeatherService::Icons::Snow:
        case Controllers::SimpleWeatherService::Icons::Thunderstorm:
          color = colorWeatherLightBlue;
          break;
        default:
          color = colorWeatherDefault;
          break;
      }
      lv_obj_set_style_local_text_color(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
      lv_label_set_text_static(weatherIcon, Symbols::GetSymbol(optCurrentWeather->iconId, weatherService.IsNight()));

      if (!weatherDisplayed) { // adjust the layout when necessary
        lv_obj_align(weatherValue, nullptr, LV_ALIGN_IN_TOP_LEFT, 200, 186);
        lv_obj_align(weatherCaption, nullptr, LV_ALIGN_IN_TOP_LEFT, 200, 210);
        lv_obj_align(weatherIcon, weatherValue, LV_ALIGN_OUT_LEFT_BOTTOM, -5, 12);
        weatherDisplayed = true;
      }
    } else {
      lv_label_set_text_static(weatherValue, "--");
      lv_label_set_text_static(weatherIcon, "");

      if (weatherDisplayed) { // adjust the layout when necessary
        lv_obj_align(weatherValue, nullptr, LV_ALIGN_IN_TOP_LEFT, 188, 186);
        lv_obj_align(weatherCaption, nullptr, LV_ALIGN_IN_TOP_LEFT, 188, 210);
        lv_obj_align(weatherIcon, weatherValue, LV_ALIGN_OUT_LEFT_BOTTOM, -5, 12);
        weatherDisplayed = false;
      }
    }
  }
}
