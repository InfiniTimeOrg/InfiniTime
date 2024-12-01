#include "displayapp/screens/Sleep.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/settings/Settings.h"
#include "components/alarm/AlarmController.h"
#include "components/motor/MotorController.h"
#include "systemtask/SystemTask.h"

#include <libraries/log/nrf_log.h>
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::InfiniSleepController;

namespace {
  void ValueChangedHandler(void* userData) {
    auto* screen = static_cast<Sleep*>(userData);
    screen->OnValueChanged();
  }
}

extern InfiniSleepController infiniSleepController;

static void settingsToggleEventHandler(lv_obj_t* obj, lv_event_t e) {
  if (e != LV_EVENT_VALUE_CHANGED) {
    return;
  }

  const char* setting_name = static_cast<const char*>(obj->user_data);
  bool enabled = lv_checkbox_is_checked(obj);

  if (strcmp(setting_name, "Body Tracking") == 0) {
    infiniSleepController.SetBodyTrackingEnabled(enabled);
    infiniSleepController.SetSettingsChanged();
  } else if (strcmp(setting_name, "Heart Rate\nTracking") == 0) {
    infiniSleepController.SetHeartRateTrackingEnabled(enabled);
    infiniSleepController.SetSettingsChanged();
  } else if (strcmp(setting_name, "Gradual Wake") == 0) {
    infiniSleepController.SetGradualWakeEnabled(enabled);
    infiniSleepController.SetSettingsChanged();
  } else if (strcmp(setting_name, "Smart Alarm\n(alpha)") == 0) {
    infiniSleepController.SetSmartAlarmEnabled(enabled);
    infiniSleepController.SetSettingsChanged();
  }
}

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Sleep*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

static void SnoozeAlarmTaskCallback(lv_task_t* task) {
  auto* screen = static_cast<Sleep*>(task->user_data);
  lv_task_set_prio(task, LV_TASK_PRIO_OFF);
  screen->StopAlerting(false);
  screen->UpdateDisplay();
  screen->SnoozeWakeAlarm();
}

static void PressesToStopAlarmTimeoutCallback(lv_task_t* task) {
  auto* screen = static_cast<Sleep*>(task->user_data);
  screen->infiniSleepController.pushesLeftToStopWakeAlarm = PUSHES_TO_STOP_ALARM;
  screen->UpdateDisplay();
}

Sleep::Sleep(Controllers::InfiniSleepController& infiniSleepController,
             Controllers::Settings::ClockType clockType,
             System::SystemTask& systemTask,
             Controllers::MotorController& motorController,
             DisplayApp& displayApp)
  : infiniSleepController {infiniSleepController},
    wakeLock(systemTask),
    motorController {motorController},
    clockType {clockType},
    displayApp {displayApp} {

  infiniSleepController.SetHeartRateTrackingEnabled(false);
  infiniSleepController.SetSettingsChanged();
  UpdateDisplay();
  taskRefresh = lv_task_create(RefreshTaskCallback, 2000, LV_TASK_PRIO_MID, this);
  taskPressesToStopAlarmTimeout =
    lv_task_create(PressesToStopAlarmTimeoutCallback, PUSHES_TO_STOP_ALARM_TIMEOUT * 1000, LV_TASK_PRIO_MID, this);
  infiniSleepController.infiniSleepSettings.sleepCycleDuration = 90;
  infiniSleepController.SetSettingsChanged();
}

Sleep::~Sleep() {
  if (infiniSleepController.IsAlerting()) {
    StopAlerting();
  }
  lv_task_del(taskRefresh);
  lv_task_del(taskPressesToStopAlarmTimeout);
  if (taskSnoozeWakeAlarm != nullptr) {
    lv_task_del(taskSnoozeWakeAlarm);
  }
  lv_obj_clean(lv_scr_act());
  infiniSleepController.SaveWakeAlarm();
  infiniSleepController.SaveInfiniSleepSettings();
}

void Sleep::DisableWakeAlarm() {
  if (infiniSleepController.GetWakeAlarm().isEnabled) {
    infiniSleepController.DisableWakeAlarm();
    lv_switch_off(enableSwitch, LV_ANIM_ON);
  }
}

void Sleep::Refresh() {
  UpdateDisplay();
}

void Sleep::UpdateDisplay() {
  // Clear the screen
  lv_obj_clean(lv_scr_act());
  if (infiniSleepController.IsAlerting()) {
    displayState = SleepDisplayState::Alarm;
  }
  // Draw the screen
  switch (displayState) {
    case SleepDisplayState::Alarm:
      DrawAlarmScreen();
      pageIndicator2.Create();
      break;
    case SleepDisplayState::Info:
      DrawInfoScreen();
      pageIndicator1.Create();
      break;
    case SleepDisplayState::Settings:
      DrawSettingsScreen();
      pageIndicator3.Create();
      break;
  }
}

void Sleep::DrawAlarmScreen() {
  hourCounter.Create();
  lv_obj_align(hourCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  if (clockType == Controllers::Settings::ClockType::H12) {
    hourCounter.EnableTwelveHourMode();

    lblampm = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_font(lblampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
    lv_label_set_text_static(lblampm, "AM");
    lv_label_set_align(lblampm, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(lblampm, lv_scr_act(), LV_ALIGN_CENTER, 0, 30);
  }
  hourCounter.SetValue(infiniSleepController.GetWakeAlarm().hours);
  hourCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  minuteCounter.Create();
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  minuteCounter.SetValue(infiniSleepController.GetWakeAlarm().minutes);
  minuteCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  lv_obj_t* colonLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align(colonLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -29);

  if (infiniSleepController.IsAlerting()) {
    lv_obj_set_hidden(hourCounter.GetObject(), true);
    lv_obj_set_hidden(minuteCounter.GetObject(), true);
    lv_obj_set_hidden(colonLabel, true);

    lv_obj_t* lblTime = lv_label_create(lv_scr_act(), nullptr);
    if (clockType == Controllers::Settings::ClockType::H24) {
      lv_label_set_text_fmt(lblTime, "%02d:%02d", infiniSleepController.GetCurrentHour(), infiniSleepController.GetCurrentMinute());
    } else {
      lv_label_set_text_fmt(lblTime, "%02d:%02d", (infiniSleepController.GetCurrentHour() % 12 == 0) ? 12 : infiniSleepController.GetCurrentHour() % 12, infiniSleepController.GetCurrentMinute());
    }
    lv_obj_align(lblTime, lv_scr_act(), LV_ALIGN_CENTER, -87, -100);
    lv_obj_set_style_local_text_color(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_label_set_align(lblTime, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_text_font(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);

    lv_obj_t* lblWaketxt = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(lblWaketxt, "Wake Up!");
    lv_obj_align(lblWaketxt, lv_scr_act(), LV_ALIGN_CENTER, 0, -22);
    lv_label_set_align(lblWaketxt, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_text_font(lblWaketxt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
    lv_obj_set_style_local_text_color(lblWaketxt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  }

  btnSnooze = lv_btn_create(lv_scr_act(), nullptr);
  btnSnooze->user_data = this;
  lv_obj_set_event_cb(btnSnooze, btnEventHandler);
  lv_obj_set_size(btnSnooze, 200, 63);
  lv_obj_align(btnSnooze, lv_scr_act(), LV_ALIGN_CENTER, 0, 28);
  lv_obj_set_style_local_bg_color(btnSnooze, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  txtSnooze = lv_label_create(btnSnooze, nullptr);
  lv_label_set_text_static(txtSnooze, "Snooze");
  lv_obj_set_hidden(btnSnooze, true);

  btnStop = lv_btn_create(lv_scr_act(), nullptr);
  btnStop->user_data = this;
  lv_obj_set_event_cb(btnStop, btnEventHandler);
  lv_obj_set_size(btnStop, 130, 50);
  lv_obj_align(btnStop, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_local_bg_color(btnStop, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  txtStop = lv_label_create(btnStop, nullptr);
  lv_label_set_text_fmt(txtStop, "Stop: %d/%d", PUSHES_TO_STOP_ALARM - infiniSleepController.pushesLeftToStopWakeAlarm, PUSHES_TO_STOP_ALARM);
  lv_obj_set_hidden(btnStop, true);

  static constexpr lv_color_t bgColor = Colors::bgAlt;

  btnSuggestedAlarm = lv_btn_create(lv_scr_act(), nullptr);
  btnSuggestedAlarm->user_data = this;
  lv_obj_set_event_cb(btnSuggestedAlarm, btnEventHandler);
  lv_obj_set_size(btnSuggestedAlarm, 115, 50);
  lv_obj_align(btnSuggestedAlarm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  // txtSuggestedAlarm = lv_label_create(btnSuggestedAlarm, nullptr);
  // lv_label_set_text_static(txtSuggestedAlarm, "Use Sugg.\nAlarmTime");

  txtSuggestedAlarm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(txtSuggestedAlarm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -15, -10);
  lv_label_set_text_static(txtSuggestedAlarm, "Auto");

  iconSuggestedAlarm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(iconSuggestedAlarm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_align(iconSuggestedAlarm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -50, -10);
  lv_label_set_text_static(iconSuggestedAlarm, Symbols::sun);

  enableSwitch = lv_switch_create(lv_scr_act(), nullptr);
  enableSwitch->user_data = this;
  lv_obj_set_event_cb(enableSwitch, btnEventHandler);
  lv_obj_set_size(enableSwitch, 100, 50);
  // Align to the center of 115px from edge
  lv_obj_align(enableSwitch, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 7, 0);
  lv_obj_set_style_local_bg_color(enableSwitch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, bgColor);

  UpdateWakeAlarmTime();

  if (alreadyAlerting) {
    RedrawSetAlerting();
    return;
  }

  if (infiniSleepController.IsAlerting()) {
    SetAlerting();
  } else {
    SetSwitchState(LV_ANIM_OFF);
  }
}

void Sleep::DrawInfoScreen() {
  lv_obj_t* lblTime = lv_label_create(lv_scr_act(), nullptr);
  if (clockType == Controllers::Settings::ClockType::H24) {
    lv_label_set_text_fmt(lblTime,
                          "%02d:%02d",
                          infiniSleepController.GetCurrentHour(),
                          infiniSleepController.GetCurrentMinute());
  } else {
    lv_label_set_text_fmt(lblTime,
                          "%02d:%02d",
                          (infiniSleepController.GetCurrentHour() % 12 == 0) ? 12 : infiniSleepController.GetCurrentHour() % 12,
                          infiniSleepController.GetCurrentMinute());
  }
  lv_obj_align(lblTime, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 5);
  lv_obj_set_style_local_text_color(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  // Total sleep time
  label_total_sleep = lv_label_create(lv_scr_act(), nullptr);

  uint16_t totalMinutes = infiniSleepController.GetTotalSleep();

  lv_label_set_text_fmt(label_total_sleep, "Time Asleep: %dh%dm", totalMinutes / 60, totalMinutes % 60);
  lv_obj_align(label_total_sleep, lv_scr_act(), LV_ALIGN_CENTER, 0, -60);
  lv_obj_set_style_local_text_color(label_total_sleep,
                                    LV_LABEL_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    infiniSleepController.IsEnabled() ? LV_COLOR_RED : LV_COLOR_WHITE);

  // Sleep Cycles Info
  label_sleep_cycles = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(label_sleep_cycles,
                        "Sleep Cycles: %d.%02d",
                        infiniSleepController.GetSleepCycles() / 100,
                        infiniSleepController.GetSleepCycles() % 100);
  lv_obj_align(label_sleep_cycles, lv_scr_act(), LV_ALIGN_CENTER, 0, -40);
  lv_obj_set_style_local_text_color(label_sleep_cycles,
                                    LV_LABEL_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    infiniSleepController.IsEnabled() ? LV_COLOR_RED : LV_COLOR_WHITE);

  // Start time
  if (infiniSleepController.IsEnabled()) {
    label_start_time = lv_label_create(lv_scr_act(), nullptr);
    if (clockType == Controllers::Settings::ClockType::H24) {
      lv_label_set_text_fmt(label_start_time,
                            "Began at: %02d:%02d",
                            infiniSleepController.prevSessionData.startTimeHours,
                            infiniSleepController.prevSessionData.startTimeMinutes);
    } else {
      lv_label_set_text_fmt(label_start_time,
                            "Began at: %02d:%02d",
                            (infiniSleepController.prevSessionData.startTimeHours % 12 == 0) ? 12 : infiniSleepController.prevSessionData.startTimeHours % 12,
                            infiniSleepController.prevSessionData.startTimeMinutes);
    }
    lv_obj_align(label_start_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_text_color(label_start_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  }

  // The alarm info
  label_alarm_time = lv_label_create(lv_scr_act(), nullptr);
  if (infiniSleepController.GetWakeAlarm().isEnabled) {
    if (clockType == Controllers::Settings::ClockType::H24) {
      lv_label_set_text_fmt(label_alarm_time,
                            "Alarm at: %02d:%02d",
                            infiniSleepController.GetWakeAlarm().hours,
                            infiniSleepController.GetWakeAlarm().minutes);
    } else {
      lv_label_set_text_fmt(label_alarm_time,
                            "Alarm at: %02d:%02d",
                            (infiniSleepController.GetWakeAlarm().hours % 12 == 0) ? 12 : infiniSleepController.GetWakeAlarm().hours % 12,
                            infiniSleepController.GetWakeAlarm().minutes);
    }
  } else {
    lv_label_set_text_static(label_alarm_time, "Alarm is not set.");
  }
  lv_obj_align(label_alarm_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 20);
  lv_obj_set_style_local_text_color(label_alarm_time,
                                    LV_LABEL_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    infiniSleepController.IsEnabled() ? LV_COLOR_RED : LV_COLOR_WHITE);

  // Gradual Wake info
  label_gradual_wake = lv_label_create(lv_scr_act(), nullptr);
  if (infiniSleepController.GetInfiniSleepSettings().graddualWake && infiniSleepController.gradualWakeStep >= 0) {
    lv_label_set_text_fmt(label_gradual_wake, "Gradual Wake: ON");
  } else {
    lv_label_set_text_static(label_gradual_wake, "Gradual Wake: OFF");
  }
  lv_obj_align(label_gradual_wake, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_local_text_color(label_gradual_wake,
                                    LV_LABEL_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    infiniSleepController.IsEnabled() ? LV_COLOR_RED : LV_COLOR_WHITE);

  // Start/Stop button
  trackerToggleBtn = lv_btn_create(lv_scr_act(), nullptr);
  trackerToggleBtn->user_data = this;
  lv_obj_set_height(trackerToggleBtn, 50);
  lv_obj_align(trackerToggleBtn, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  // Tracker toggle button
  trackerToggleLabel = lv_label_create(trackerToggleBtn, nullptr);
  if (infiniSleepController.IsTrackerEnabled()) {
    lv_label_set_text_static(trackerToggleLabel, "Stop");
    lv_obj_set_style_local_bg_color(trackerToggleBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else {
    lv_label_set_text_static(trackerToggleLabel, "Start");
    lv_obj_set_style_local_bg_color(trackerToggleBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  }
  lv_obj_set_event_cb(trackerToggleBtn, btnEventHandler);
}

void Sleep::DrawSettingsScreen() {
  lv_obj_t* lblSettings = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblSettings, "Settings");
  lv_obj_align(lblSettings, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  struct Setting {
    const char* name;
    bool enabled;
    int offsetAfter = 30;
  };

  const Setting settings[] = {
    //{"Body Tracking", infiniSleepController.BodyTrackingEnabled()},
    // {"Heart Rate\nTracking", infiniSleepController.HeartRateTrackingEnabled(), 60},
    {"Gradual Wake", infiniSleepController.GradualWakeEnabled()},
    //{"Smart Alarm\n(alpha)", infiniSleepController.SmartAlarmEnabled()}
  };

  int y_offset = 50;
  for (const auto& setting : settings) {

    lv_obj_t* checkbox = lv_checkbox_create(lv_scr_act(), nullptr);
    checkbox->user_data = const_cast<char*>(setting.name);
    lv_checkbox_set_text_static(checkbox, const_cast<char*>(setting.name));

    if (setting.enabled) {
      lv_checkbox_set_checked(checkbox, true);
    } else {
      lv_checkbox_set_checked(checkbox, false);
    }
    lv_obj_align(checkbox, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, y_offset);

    lv_obj_set_event_cb(checkbox, settingsToggleEventHandler);
    y_offset += setting.offsetAfter; // Increase the offset to provide better spacing
  }

  lv_obj_t* lblCycles = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblCycles, "Desired\nCycles");
  lv_obj_align(lblCycles, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, y_offset);

  lv_obj_t* btnCycles = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btnCycles, 100, 50);
  lv_obj_align(btnCycles, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 130, y_offset);
  btnCycles->user_data = this;
  lv_obj_set_event_cb(btnCycles, [](lv_obj_t* obj, lv_event_t e) {
    if (e == LV_EVENT_CLICKED) {
      auto* screen = static_cast<Sleep*>(obj->user_data);
      int value = screen->infiniSleepController.infiniSleepSettings.desiredCycles;
      value = (value % 10) + 1; // Cycle through values 1 to 10
      screen->infiniSleepController.infiniSleepSettings.desiredCycles = value;
      screen->infiniSleepController.SetSettingsChanged();
      lv_label_set_text_fmt(lv_obj_get_child(obj, nullptr), "%d", value);
    }
  });

  lv_obj_t* lblCycleValue = lv_label_create(btnCycles, nullptr);
  lv_label_set_text_fmt(lblCycleValue, "%d", infiniSleepController.infiniSleepSettings.desiredCycles);
  lv_obj_align(lblCycleValue, nullptr, LV_ALIGN_CENTER, 0, 0);

  infiniSleepController.infiniSleepSettings.sleepCycleDuration = 90;
  infiniSleepController.SetSettingsChanged();

  y_offset += 70; // Adjust the offset for the next UI element
}

void Sleep::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnSnooze) {
      StopAlerting();
      UpdateDisplay();
      SnoozeWakeAlarm();
      return;
    }
    if (obj == btnStop) {
      StopAlarmPush();
      return;
    }
    if (obj == btnMessage) {
      HideAlarmInfo();
      return;
    }
    if (obj == enableSwitch) {
      if (lv_switch_get_state(enableSwitch)) {
        infiniSleepController.ScheduleWakeAlarm();
      } else {
        infiniSleepController.DisableWakeAlarm();
      }
      return;
    }
    if (obj == trackerToggleBtn) {
      infiniSleepController.ToggleTracker();
      UpdateDisplay();
      return;
    }
    if (obj == btnSuggestedAlarm) {
      // Set the suggested time
      uint16_t totalSuggestedMinutes = infiniSleepController.GetSuggestedSleepTime();
      uint8_t suggestedHours = totalSuggestedMinutes / 60;
      uint8_t suggestedMinutes = totalSuggestedMinutes % 60;

      // Time for alarm, current time + suggested sleep time
      uint8_t alarmHour = (infiniSleepController.GetCurrentHour() + suggestedHours) % 24;
      uint8_t alarmMinute = (infiniSleepController.GetCurrentMinute() + suggestedMinutes) % 60;

      infiniSleepController.SetWakeAlarmTime(alarmHour, alarmMinute);

      hourCounter.SetValue(alarmHour);
      minuteCounter.SetValue(alarmMinute);

      OnValueChanged();
      infiniSleepController.ScheduleWakeAlarm();
      return;
    }
  }
}

bool Sleep::OnButtonPushed() {
  if (txtMessage != nullptr && btnMessage != nullptr) {
    HideAlarmInfo();
    return true;
  }
  if (infiniSleepController.IsAlerting()) {
    if (StopAlarmPush()) {
      return true;
    }
  }
  return false;
}

bool Sleep::StopAlarmPush() {
  if (infiniSleepController.pushesLeftToStopWakeAlarm > 1) {
    lv_task_reset(taskPressesToStopAlarmTimeout);
    infiniSleepController.pushesLeftToStopWakeAlarm--;
    UpdateDisplay();
    return true;
  } else {
    if (infiniSleepController.isSnoozing) {
      infiniSleepController.RestorePreSnoozeTime();
    }
    infiniSleepController.isSnoozing = false;
    StopAlerting();
    if (infiniSleepController.IsTrackerEnabled()) {
      displayState = SleepDisplayState::Info;
      UpdateDisplay();
      OnButtonEvent(trackerToggleBtn, LV_EVENT_CLICKED);
      return true;
    }
    displayState = SleepDisplayState::Info;
    UpdateDisplay();
    return true;
  }
}

bool Sleep::OnTouchEvent(Pinetime::Applications::TouchEvents event) {

  // Swiping should be ignored when in alerting state
  if (infiniSleepController.IsAlerting() && (event != TouchEvents::SwipeDown && event != TouchEvents::SwipeUp &&
                                             event != TouchEvents::SwipeLeft && event != TouchEvents::SwipeRight)) {
    return true;
  }

  lastDisplayState = displayState;
  NRF_LOG_INFO("Last Display State: %d", static_cast<uint8_t>(lastDisplayState));

  // The cases for swiping to change page on app
  switch (event) {
    case TouchEvents::SwipeDown:
      if (static_cast<uint8_t>(displayState) != 0) {
        displayApp.SetFullRefresh(Pinetime::Applications::DisplayApp::FullRefreshDirections::Down);
        displayState = static_cast<SleepDisplayState>(static_cast<int>(displayState) - 1);
        UpdateDisplay();
      } else {
        return false;
      }
      NRF_LOG_INFO("SwipeDown: %d", static_cast<uint8_t>(displayState));
      return true;
    case TouchEvents::SwipeUp:
      if (static_cast<uint8_t>(displayState) != 2) {
        displayApp.SetFullRefresh(Pinetime::Applications::DisplayApp::FullRefreshDirections::Up);
        displayState = static_cast<SleepDisplayState>(static_cast<uint8_t>(displayState) + 1);
        UpdateDisplay();
      }
      NRF_LOG_INFO("SwipeUp: %d", static_cast<uint8_t>(displayState));
      return true;
    default:
      break;
  }

  // Don't allow closing the screen by swiping while the alarm is alerting
  return infiniSleepController.IsAlerting() && event == TouchEvents::SwipeDown;
}

void Sleep::OnValueChanged() {
  DisableWakeAlarm();
  UpdateWakeAlarmTime();
}

// Currently snoozes baeed on define statement in InfiniSleepController.h
void Sleep::SnoozeWakeAlarm() {
  if (taskSnoozeWakeAlarm != nullptr) {
    lv_task_del(taskSnoozeWakeAlarm);
    taskSnoozeWakeAlarm = nullptr;
  }

  NRF_LOG_INFO("Snoozing alarm for %d minutes", SNOOZE_MINUTES);

  uint16_t totalAlarmMinutes = infiniSleepController.GetCurrentHour() * 60 + infiniSleepController.GetCurrentMinute();
  uint16_t newSnoozeMinutes = totalAlarmMinutes + SNOOZE_MINUTES;

  infiniSleepController.SetPreSnoozeTime();
  infiniSleepController.isSnoozing = true;

  infiniSleepController.SetWakeAlarmTime(newSnoozeMinutes / 60, newSnoozeMinutes % 60);

  hourCounter.SetValue(newSnoozeMinutes / 60);
  minuteCounter.SetValue(newSnoozeMinutes % 60);

  infiniSleepController.ScheduleWakeAlarm();
}

void Sleep::UpdateWakeAlarmTime() {
  if (lblampm != nullptr) {
    if (hourCounter.GetValue() >= 12) {
      lv_label_set_text_static(lblampm, "PM");
    } else {
      lv_label_set_text_static(lblampm, "AM");
    }
  }
  infiniSleepController.SetWakeAlarmTime(hourCounter.GetValue(), minuteCounter.GetValue());
  SetSwitchState(LV_ANIM_OFF);
}

void Sleep::SetAlerting() {
  lv_obj_set_hidden(enableSwitch, true);
  lv_obj_set_hidden(btnSnooze, false);
  lv_obj_set_hidden(btnStop, false);
  lv_obj_set_hidden(btnSuggestedAlarm, true);
  lv_obj_set_hidden(txtSuggestedAlarm, true);
  lv_obj_set_hidden(iconSuggestedAlarm, true);
  NRF_LOG_INFO("Alarm is alerting");
  taskSnoozeWakeAlarm = lv_task_create(SnoozeAlarmTaskCallback, 120 * 1000, LV_TASK_PRIO_MID, this);
  motorController.StartWakeAlarm();
  wakeLock.Lock();
  alreadyAlerting = true;
}

void Sleep::RedrawSetAlerting() {
  lv_obj_set_hidden(enableSwitch, true);
  lv_obj_set_hidden(btnSnooze, false);
  lv_obj_set_hidden(btnStop, false);
  lv_obj_set_hidden(btnSuggestedAlarm, true);
  lv_obj_set_hidden(txtSuggestedAlarm, true);
  lv_obj_set_hidden(iconSuggestedAlarm, true);
  wakeLock.Lock();
}

void Sleep::StopAlerting(bool setSwitch) {
  infiniSleepController.StopAlerting();
  motorController.StopWakeAlarm();
  if (setSwitch) {
    SetSwitchState(LV_ANIM_OFF);
  }
  wakeLock.Release();
  lv_obj_set_hidden(enableSwitch, false);
  lv_obj_set_hidden(btnSnooze, true);
  lv_obj_set_hidden(btnStop, true);
  lv_obj_set_hidden(btnSuggestedAlarm, false);
  lv_obj_set_hidden(txtSuggestedAlarm, false);
  lv_obj_set_hidden(iconSuggestedAlarm, false);
  alreadyAlerting = false;
}

void Sleep::SetSwitchState(lv_anim_enable_t anim) {
  if (infiniSleepController.GetWakeAlarm().isEnabled) {
    lv_switch_on(enableSwitch, anim);
  } else {
    lv_switch_off(enableSwitch, anim);
  }
}

void Sleep::ShowAlarmInfo() {
  if (btnMessage != nullptr) {
    return;
  }
  btnMessage = lv_btn_create(lv_scr_act(), nullptr);
  btnMessage->user_data = this;
  lv_obj_set_event_cb(btnMessage, btnEventHandler);
  lv_obj_set_height(btnMessage, 200);
  lv_obj_set_width(btnMessage, 150);
  lv_obj_align(btnMessage, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  txtMessage = lv_label_create(btnMessage, nullptr);
  lv_obj_set_style_local_bg_color(btnMessage, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_NAVY);

  if (infiniSleepController.GetWakeAlarm().isEnabled) {
    auto timeToAlarm = infiniSleepController.SecondsToWakeAlarm();

    auto daysToAlarm = timeToAlarm / 86400;
    auto hrsToAlarm = (timeToAlarm % 86400) / 3600;
    auto minToAlarm = (timeToAlarm % 3600) / 60;
    auto secToAlarm = timeToAlarm % 60;

    lv_label_set_text_fmt(txtMessage,
                          "Time to\nalarm:\n%2lu Days\n%2lu Hours\n%2lu Minutes\n%2lu Seconds",
                          daysToAlarm,
                          hrsToAlarm,
                          minToAlarm,
                          secToAlarm);
  } else {
    lv_label_set_text_static(txtMessage, "Alarm\nis not\nset.");
  }
}

void Sleep::HideAlarmInfo() {
  lv_obj_del(btnMessage);
  txtMessage = nullptr;
  btnMessage = nullptr;
}