#include "displayapp/screens/WatchFaceNetRunner.h"

#include <lvgl/lvgl.h>
#include "Identity.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  lv_obj_t* CreateLabel(lv_obj_t* parent, lv_color_t color) {
    lv_obj_t* label = lv_label_create(parent, nullptr);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
    return label;
  }

  lv_color_t PowerColor(int percent) {
    if (percent > 50) {
      return Colors::neonGreen;
    }
    if (percent > 20) {
      return Colors::neonYellow;
    }
    return Colors::neonMagenta;
  }
}

WatchFaceNetRunner::WatchFaceNetRunner(Controllers::DateTime& dateTimeController,
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

  // HUD frame lines
  topLine = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(topLine, topLinePoints, 4);
  lv_obj_set_style_local_line_color(topLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, Colors::neonMagenta);
  lv_obj_set_style_local_line_width(topLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 2);

  bottomLine = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(bottomLine, bottomLinePoints, 4);
  lv_obj_set_style_local_line_color(bottomLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, Colors::neonCyan);
  lv_obj_set_style_local_line_width(bottomLine, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 2);

  // Status row
  labelLink = CreateLabel(lv_scr_act(), Colors::gray);
  lv_obj_set_pos(labelLink, 4, 4);

  labelPower = CreateLabel(lv_scr_act(), Colors::neonGreen);
  lv_obj_align(labelPower, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 4);

  // Date row
  labelDate = CreateLabel(lv_scr_act(), Colors::neonYellow);
  lv_obj_set_pos(labelDate, 2, 38);

  labelNotification = CreateLabel(lv_scr_act(), Colors::neonMagenta);
  lv_label_set_text_static(labelNotification, "[MSG]");
  lv_obj_align(labelNotification, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 44);
  lv_obj_set_hidden(labelNotification, true);

  // Time: a yellow ghost and a magenta shadow sit behind the cyan digits for the
  // chromatic-aberration look. The ghost only shows up while glitching.
  labelTimeGhost = CreateLabel(lv_scr_act(), Colors::neonYellow);
  labelTimeShadow = CreateLabel(lv_scr_act(), Colors::neonMagenta);
  labelTime = CreateLabel(lv_scr_act(), Colors::neonCyan);
  for (lv_obj_t* label : {labelTimeGhost, labelTimeShadow, labelTime}) {
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
    lv_obj_set_style_local_text_letter_space(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, -6);
    lv_label_set_text_static(label, "00:00");
  }
  lv_obj_set_hidden(labelTimeGhost, true);

  glitchSlice = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(glitchSlice, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Colors::neonMagenta);
  lv_obj_set_style_local_radius(glitchSlice, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_border_width(glitchSlice, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_hidden(glitchSlice, true);

  // Seconds sweep
  barSeconds = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(barSeconds, 150, 6);
  lv_obj_set_pos(barSeconds, 6, 138);
  lv_bar_set_range(barSeconds, 0, 59);
  lv_obj_set_style_local_bg_color(barSeconds, LV_BAR_PART_BG, LV_STATE_DEFAULT, Colors::dimCyan);
  lv_obj_set_style_local_bg_opa(barSeconds, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_radius(barSeconds, LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_bg_color(barSeconds, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::neonYellow);
  lv_obj_set_style_local_radius(barSeconds, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 0);

  labelSeconds = CreateLabel(lv_scr_act(), Colors::neonYellow);
  lv_obj_set_pos(labelSeconds, 164, 129);

  // Biometrics and environment
  labelVitals = CreateLabel(lv_scr_act(), LV_COLOR_WHITE);
  lv_label_set_recolor(labelVitals, true);
  lv_obj_set_pos(labelVitals, 4, 148);

  labelEnv = CreateLabel(lv_scr_act(), LV_COLOR_WHITE);
  lv_label_set_recolor(labelEnv, true);
  lv_obj_set_pos(labelEnv, 4, 170);

  // Footer
  labelHandle = CreateLabel(lv_scr_act(), Colors::neonMagenta);
  lv_label_set_text_static(labelHandle, Pinetime::Identity::handle);
  lv_obj_set_pos(labelHandle, 4, 212);

  labelHex = CreateLabel(lv_scr_act(), Colors::neonPurple);
  lv_obj_align(labelHex, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 212);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceNetRunner::~WatchFaceNetRunner() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

uint32_t WatchFaceNetRunner::NextRandom() {
  // xorshift32
  rngState ^= rngState << 13;
  rngState ^= rngState >> 17;
  rngState ^= rngState << 5;
  return rngState;
}

void WatchFaceNetRunner::StartGlitch() {
  glitchFrames = 4 + NextRandom() % 5;
  lv_obj_set_hidden(labelTimeGhost, false);
  lv_obj_set_hidden(glitchSlice, false);
}

void WatchFaceNetRunner::UpdateGlitch() {
  if (glitchFrames == 0) {
    return;
  }
  glitchFrames--;

  const lv_coord_t timeX = (LV_HOR_RES - lv_obj_get_width(labelTime)) / 2;
  if (glitchFrames == 0) {
    lv_obj_set_hidden(labelTimeGhost, true);
    lv_obj_set_hidden(glitchSlice, true);
    lv_obj_set_pos(labelTime, timeX, timeY);
    lv_obj_set_pos(labelTimeShadow, timeX + shadowOffset, timeY + shadowOffset);
    return;
  }

  auto jitter = [this](int range) {
    return static_cast<lv_coord_t>(static_cast<int>(NextRandom() % (2 * range + 1)) - range);
  };
  lv_obj_set_pos(labelTime, timeX + jitter(3), timeY);
  lv_obj_set_pos(labelTimeShadow, timeX + shadowOffset + jitter(8), timeY + shadowOffset);
  lv_obj_set_pos(labelTimeGhost, timeX + jitter(10), timeY + jitter(3));

  lv_obj_set_style_local_bg_color(glitchSlice,
                                  LV_OBJ_PART_MAIN,
                                  LV_STATE_DEFAULT,
                                  (NextRandom() & 1) != 0 ? Colors::neonMagenta : Colors::neonCyan);
  lv_obj_set_size(glitchSlice, 40 + NextRandom() % 160, 2 + NextRandom() % 5);
  lv_obj_set_pos(glitchSlice, NextRandom() % 120, timeY + NextRandom() % 60);
}

void WatchFaceNetRunner::Refresh() {
  UpdateGlitch();

  notificationState = notificationManager.AreNewNotificationsAvailable();

  currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated()) {
    const uint8_t second = dateTimeController.Seconds();
    lv_bar_set_value(barSeconds, second, LV_ANIM_OFF);

    const auto epoch = std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.UTCDateTime().time_since_epoch()).count();
    lv_label_set_text_fmt(labelHex, "0x%08lX", static_cast<unsigned long>(epoch));
    lv_obj_align(labelHex, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 212);

    // Blink the notification tag once per second
    lv_obj_set_hidden(labelNotification, !notificationState.Get() || (second % 2) != 0);

    // Stir the RNG with the clock so every boot glitches differently, then roll for a glitch
    rngState ^= static_cast<uint32_t>(epoch);
    if (rngState == 0) {
      rngState = 0x1337c0de;
    }
    if (glitchFrames == 0 && NextRandom() % 7 == 0) {
      StartGlitch();
    }

    currentMinute = std::chrono::time_point_cast<std::chrono::minutes>(currentDateTime.Get());
    if (currentMinute.IsUpdated()) {
      uint8_t hour = dateTimeController.Hours();
      const uint8_t minute = dateTimeController.Minutes();

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        amPm = hour < 12 ? "AM" : "PM";
        if (hour == 0) {
          hour = 12;
        } else if (hour > 12) {
          hour -= 12;
        }
      } else {
        amPm = "";
      }

      for (lv_obj_t* label : {labelTimeGhost, labelTimeShadow, labelTime}) {
        lv_label_set_text_fmt(label, "%02d:%02d", hour, minute);
      }
      const lv_coord_t timeX = (LV_HOR_RES - lv_obj_get_width(labelTime)) / 2;
      lv_obj_set_pos(labelTime, timeX, timeY);
      lv_obj_set_pos(labelTimeShadow, timeX + shadowOffset, timeY + shadowOffset);
      lv_obj_set_pos(labelTimeGhost, timeX, timeY);

      // A guaranteed glitch at the top of every minute
      StartGlitch();
    }

    lv_label_set_text_fmt(labelSeconds, ":%02d%s", second, amPm);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      lv_label_set_text_fmt(labelDate,
                            "%s %04d.%02d.%02d",
                            dateTimeController.DayOfWeekShortToString(),
                            dateTimeController.Year(),
                            static_cast<int>(dateTimeController.Month()),
                            dateTimeController.Day());
    }
  }

  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    const int percent = batteryPercentRemaining.Get();
    lv_obj_set_style_local_text_color(labelPower, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, PowerColor(percent));
    if (batteryController.IsCharging()) {
      lv_label_set_text_fmt(labelPower, "CHG %d%%", percent);
    } else {
      lv_label_set_text_fmt(labelPower, "PWR %d%%", percent);
    }
    lv_obj_align(labelPower, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 4);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    if (!bleRadioEnabled.Get()) {
      lv_label_set_text_static(labelLink, "LNK:OFF");
      lv_obj_set_style_local_text_color(labelLink, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::gray);
    } else if (bleState.Get()) {
      lv_label_set_text_static(labelLink, "LNK:UP");
      lv_obj_set_style_local_text_color(labelLink, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::neonCyan);
    } else {
      lv_label_set_text_static(labelLink, "LNK:--");
      lv_obj_set_style_local_text_color(labelLink, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    }
  }

  stepCount = motionController.NbSteps();
  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (stepCount.IsUpdated() || heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get() && heartbeat.Get() != 0) {
      lv_label_set_text_fmt(labelVitals, "#ff2a6d HR# %03d  #05d9e8 STP# %lu", heartbeat.Get(), stepCount.Get());
    } else {
      lv_label_set_text_fmt(labelVitals, "#ff2a6d HR# ---  #05d9e8 STP# %lu", stepCount.Get());
    }
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
      lv_label_set_text_fmt(labelEnv, "#9d4edd ENV# %d°%c %s", temp, tempUnit, Symbols::GetSimpleCondition(optCurrentWeather->iconId));
    } else {
      lv_label_set_text_static(labelEnv, "#9d4edd ENV# NO SIGNAL");
    }
  }
}
