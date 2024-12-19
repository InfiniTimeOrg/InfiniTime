#include "displayapp/screens/Sleep.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "components/settings/Settings.h"
#include "components/infinisleep/InfiniSleepController.h"
#include "components/motor/MotorController.h"
#include "systemtask/SystemTask.h"

#include <libraries/log/nrf_log.h>
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

namespace {
  void ValueChangedHandler(void* userData) {
    auto* screen = static_cast<Sleep*>(userData);
    screen->OnValueChanged();
  }

  void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Sleep*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }

  void SnoozeAlarmTaskCallback(lv_task_t* task) {
    lv_task_set_prio(task, LV_TASK_PRIO_OFF);
    auto* screen = static_cast<Sleep*>(task->user_data);
    screen->ignoreButtonPush = true;
    screen->StopAlerting(false);
    screen->UpdateDisplay();
    screen->SnoozeWakeAlarm();
    screen->displayState = Sleep::SleepDisplayState::Info;
    screen->UpdateDisplay();
    screen->ignoreButtonPush = false;
  }

  void PressesToStopAlarmTimeoutCallback(lv_task_t* task) {
    auto* screen = static_cast<Sleep*>(task->user_data);
    screen->infiniSleepController.pushesLeftToStopWakeAlarm = screen->infiniSleepController.infiniSleepSettings.pushesToStopAlarm;
    screen->UpdateDisplay();
  }
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

  infiniSleepController.infiniSleepSettings.heartRateTracking = false;
  infiniSleepController.SetSettingsChanged();
  UpdateDisplay();
  taskRefresh = lv_task_create(RefreshTaskCallback, 2000, LV_TASK_PRIO_MID, this);
  taskPressesToStopAlarmTimeout =
    lv_task_create(PressesToStopAlarmTimeoutCallback, PUSHES_TO_STOP_ALARM_TIMEOUT * 1000, LV_TASK_PRIO_MID, this);
  infiniSleepController.infiniSleepSettings.sleepCycleDuration = 90;
  infiniSleepController.SetSettingsChanged();

  if (!infiniSleepController.IsEnabled()) {
    infiniSleepController.prevBrightnessLevel = infiniSleepController.GetBrightnessController().Level();
  }
  infiniSleepController.GetBrightnessController().Set(Controllers::BrightnessController::Levels::Low);
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
  if (!infiniSleepController.IsEnabled()) {
    infiniSleepController.GetBrightnessController().Set(infiniSleepController.prevBrightnessLevel);
  }
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
  if (infiniSleepController.IsAlerting() != true && lastDisplayState == displayState && displayState == SleepDisplayState::Alarm) {
    return;
  }

  lv_task_reset(taskRefresh);

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

  if (alreadyAlerting) {
    RedrawSetAlerting();
    return;
  }

  if (infiniSleepController.IsAlerting()) {
    SetAlerting();
  } else if (displayState == SleepDisplayState::Alarm) {
    SetSwitchState(LV_ANIM_OFF);
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
      lv_label_set_text_fmt(lblTime,
                            "%02d:%02d",
                            (infiniSleepController.GetCurrentHour() % 12 == 0) ? 12 : infiniSleepController.GetCurrentHour() % 12,
                            infiniSleepController.GetCurrentMinute());
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
  lv_label_set_text_fmt(txtStop,
                        "Stop: %d/%d",
                        infiniSleepController.infiniSleepSettings.pushesToStopAlarm - infiniSleepController.pushesLeftToStopWakeAlarm,
                        infiniSleepController.infiniSleepSettings.pushesToStopAlarm);
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
  lv_obj_align(txtSuggestedAlarm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -15, -13);
  lv_label_set_text_static(txtSuggestedAlarm, "Auto");

  iconSuggestedAlarm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(iconSuggestedAlarm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_align(iconSuggestedAlarm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -50, -13);
  lv_label_set_text_static(iconSuggestedAlarm, Symbols::sun);

  enableSwitch = lv_switch_create(lv_scr_act(), nullptr);
  enableSwitch->user_data = this;
  lv_obj_set_event_cb(enableSwitch, btnEventHandler);
  lv_obj_set_size(enableSwitch, 100, 50);
  // Align to the center of 115px from edge
  lv_obj_align(enableSwitch, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 7, 0);
  lv_obj_set_style_local_bg_color(enableSwitch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, bgColor);

  UpdateWakeAlarmTime();
}

void Sleep::DrawInfoScreen() {
  lv_obj_t* lblTime = lv_label_create(lv_scr_act(), nullptr);
  if (clockType == Controllers::Settings::ClockType::H24) {
    lv_label_set_text_fmt(lblTime, "%02d:%02d", infiniSleepController.GetCurrentHour(), infiniSleepController.GetCurrentMinute());
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

  const uint16_t totalMinutes = infiniSleepController.GetTotalSleep();

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
      lv_label_set_text_fmt(
        label_start_time,
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
  if (infiniSleepController.infiniSleepSettings.graddualWake && infiniSleepController.infiniSleepSettings.naturalWake) {
    lv_label_set_text_static(label_gradual_wake, "Wake Mode: Both");
  } else if (infiniSleepController.infiniSleepSettings.graddualWake) {
    lv_label_set_text_static(label_gradual_wake, "Wake Mode: PreWake");
  } else if (infiniSleepController.infiniSleepSettings.naturalWake) {
    lv_label_set_text_static(label_gradual_wake, "Wake Mode: Natural");
  } else {
    lv_label_set_text_static(label_gradual_wake, "Wake Mode: Normal");
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
  // lv_obj_t* lblSettings = lv_label_create(lv_scr_act(), nullptr);
  // lv_label_set_text_static(lblSettings, "Settings");
  // lv_obj_align(lblSettings, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  if (infiniSleepController.wakeAlarm.isEnabled) {
    lv_obj_t* lblWarning = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(lblWarning, "Disable alarm to\nchange settings.");
    lv_obj_align(lblWarning, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_local_text_color(lblWarning, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    return;
  }

  uint8_t y_offset = 10;

  lblWakeMode = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblWakeMode, "Wake\nMode");
  lv_obj_align(lblWakeMode, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, y_offset);

  btnWakeMode = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btnWakeMode, 100, 50);
  lv_obj_align(btnWakeMode, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 130, y_offset);
  btnWakeMode->user_data = this;
  lv_obj_set_event_cb(btnWakeMode, btnEventHandler);
  const char* mode = (infiniSleepController.infiniSleepSettings.graddualWake && infiniSleepController.infiniSleepSettings.naturalWake)
                       ? "Both"
                     : infiniSleepController.infiniSleepSettings.graddualWake ? "Pre."
                     : infiniSleepController.infiniSleepSettings.naturalWake  ? "Nat."
                                                                              : "Norm.";
  lblWakeModeValue = lv_label_create(btnWakeMode, nullptr);
  lv_label_set_text_static(lblWakeModeValue, mode);
  lv_obj_align(lblWakeModeValue, nullptr, LV_ALIGN_CENTER, 0, 0);

  y_offset += 60; // Adjust the offset for the next UI element

  lblCycles = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblCycles, "Desired\nCycles");
  lv_obj_align(lblCycles, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, y_offset);

  btnCycles = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btnCycles, 100, 50);
  lv_obj_align(btnCycles, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 130, y_offset);
  btnCycles->user_data = this;
  lv_obj_set_event_cb(btnCycles, btnEventHandler);

  lblCycleValue = lv_label_create(btnCycles, nullptr);
  lv_label_set_text_fmt(lblCycleValue, "%d", infiniSleepController.infiniSleepSettings.desiredCycles);
  lv_obj_align(lblCycleValue, nullptr, LV_ALIGN_CENTER, 0, 0);

  infiniSleepController.infiniSleepSettings.sleepCycleDuration = 90;
  infiniSleepController.SetSettingsChanged();

  y_offset += 60; // Adjust the offset for the next UI element

  btnTestMotorGradual = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btnTestMotorGradual, 110, 50);
  lv_obj_align(btnTestMotorGradual, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, y_offset);
  btnTestMotorGradual->user_data = this;
  lv_obj_set_event_cb(btnTestMotorGradual, btnEventHandler);

  lblMotorStrength = lv_label_create(btnTestMotorGradual, nullptr);
  lv_label_set_text_static(lblMotorStrength, "Motor\nPower");
  lv_obj_align(lblMotorStrength, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  btnMotorStrength = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btnMotorStrength, 100, 50);
  lv_obj_align(btnMotorStrength, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 130, y_offset);
  btnMotorStrength->user_data = this;
  lv_obj_set_event_cb(btnMotorStrength, btnEventHandler);

  lblMotorStrengthValue = lv_label_create(btnMotorStrength, nullptr);
  lv_label_set_text_fmt(lblMotorStrengthValue, "%d", infiniSleepController.infiniSleepSettings.motorStrength);
  motorController.infiniSleepMotorStrength = infiniSleepController.infiniSleepSettings.motorStrength;
  lv_obj_align(lblMotorStrengthValue, nullptr, LV_ALIGN_CENTER, 0, 0);

  y_offset += 60; // Adjust the offset for the next UI element

  lblPushesToStop = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lblPushesToStop, "Pushes\nto Stop");
  lv_obj_align(lblPushesToStop, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, y_offset);

  btnPushesToStop = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btnPushesToStop, 100, 50);
  lv_obj_align(btnPushesToStop, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 130, y_offset);
  btnPushesToStop->user_data = this;
  lv_obj_set_event_cb(btnPushesToStop, btnEventHandler);

  lblPushesToStopValue = lv_label_create(btnPushesToStop, nullptr);
  lv_label_set_text_fmt(lblPushesToStopValue, "%d", infiniSleepController.infiniSleepSettings.pushesToStopAlarm);
  lv_obj_align(lblPushesToStopValue, nullptr, LV_ALIGN_CENTER, 0, 0);
}

void Sleep::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnSnooze) {
      StopAlerting();
      UpdateDisplay();
      SnoozeWakeAlarm();
      displayState = SleepDisplayState::Info;
      UpdateDisplay();
      return;
    }
    if (obj == btnStop) {
      StopAlarmPush();
      return;
    }
    if (obj == enableSwitch) {
      if (lv_switch_get_state(enableSwitch)) {
        infiniSleepController.ScheduleWakeAlarm();
      } else {
        infiniSleepController.DisableWakeAlarm();
      }
      if (infiniSleepController.isSnoozing) {
        infiniSleepController.RestorePreSnoozeTime();
      }
      infiniSleepController.isSnoozing = false;
      return;
    }
    if (obj == trackerToggleBtn) {
      infiniSleepController.ToggleTracker();
      UpdateDisplay();
      return;
    }
    if (obj == btnSuggestedAlarm) {
      // Set the suggested time
      const uint16_t totalSuggestedMinutes = infiniSleepController.GetSuggestedSleepTime();
      const uint8_t suggestedHours = totalSuggestedMinutes / 60;
      const uint8_t suggestedMinutes = totalSuggestedMinutes % 60;

      // Time for alarm, current time + suggested sleep time
      const uint8_t alarmHour = (infiniSleepController.GetCurrentHour() + suggestedHours) % 24;
      const uint8_t alarmMinute = (infiniSleepController.GetCurrentMinute() + suggestedMinutes) % 60;

      infiniSleepController.SetWakeAlarmTime(alarmHour, alarmMinute);

      hourCounter.SetValue(alarmHour);
      minuteCounter.SetValue(alarmMinute);

      OnValueChanged();
      infiniSleepController.ScheduleWakeAlarm();
      SetSwitchState(LV_ANIM_OFF);
      return;
    }
    if (obj == btnWakeMode) {
      if (infiniSleepController.infiniSleepSettings.graddualWake && infiniSleepController.infiniSleepSettings.naturalWake) {
        infiniSleepController.infiniSleepSettings.graddualWake = false;
        infiniSleepController.infiniSleepSettings.naturalWake = false;
      } else if (infiniSleepController.infiniSleepSettings.graddualWake) {
        infiniSleepController.infiniSleepSettings.graddualWake = false;
        infiniSleepController.infiniSleepSettings.naturalWake = true;
      } else if (infiniSleepController.infiniSleepSettings.naturalWake) {
        infiniSleepController.infiniSleepSettings.naturalWake = true;
        infiniSleepController.infiniSleepSettings.graddualWake = true;
      } else if (!infiniSleepController.infiniSleepSettings.graddualWake && !infiniSleepController.infiniSleepSettings.naturalWake) {
        infiniSleepController.infiniSleepSettings.graddualWake = true;
        infiniSleepController.infiniSleepSettings.naturalWake = false;
      }
      infiniSleepController.SetSettingsChanged();
      const char* mode = (infiniSleepController.infiniSleepSettings.graddualWake && infiniSleepController.infiniSleepSettings.naturalWake)
                           ? "Both"
                         : infiniSleepController.infiniSleepSettings.graddualWake ? "Pre."
                         : infiniSleepController.infiniSleepSettings.naturalWake  ? "Nat."
                                                                                  : "Norm.";
      lv_label_set_text_static(lv_obj_get_child(obj, nullptr), mode);
      return;
    }
    if (obj == btnCycles) {
      uint8_t value = infiniSleepController.infiniSleepSettings.desiredCycles;
      value = (value % 10) + 1; // Cycle through values 1 to 10
      infiniSleepController.infiniSleepSettings.desiredCycles = value;
      infiniSleepController.SetSettingsChanged();
      lv_label_set_text_fmt(lv_obj_get_child(obj, nullptr), "%d", value);
      return;
    }
    if (obj == btnTestMotorGradual) {
      motorController.GradualWakeBuzz();
      return;
    }
    if (obj == btnMotorStrength) {
      uint8_t value = infiniSleepController.infiniSleepSettings.motorStrength;
      value += 25;
      if (value > 200) {
        value = 100;
      }
      infiniSleepController.infiniSleepSettings.motorStrength = value;
      infiniSleepController.SetSettingsChanged();
      lv_label_set_text_fmt(lv_obj_get_child(obj, nullptr), "%d", value);
      motorController.infiniSleepMotorStrength = value;
      motorController.GradualWakeBuzz();
      return;
    }
    if (obj == btnPushesToStop) {
      uint8_t value = infiniSleepController.infiniSleepSettings.pushesToStopAlarm;
      value = (value % 10) + 1; // Cycle through values 1 to 10
      infiniSleepController.infiniSleepSettings.pushesToStopAlarm = value;
      infiniSleepController.SetSettingsChanged();
      lv_label_set_text_fmt(lv_obj_get_child(obj, nullptr), "%d", value);
      return;
    }
  }
}

bool Sleep::OnButtonPushed() {
  if (infiniSleepController.IsAlerting() || ignoreButtonPush) {
    return true;
  }
  if (displayState != SleepDisplayState::Info) {
    displayState = SleepDisplayState::Info;
    UpdateDisplay();
    return true;
  }
  return false;
}

bool Sleep::StopAlarmPush() {
  if (infiniSleepController.pushesLeftToStopWakeAlarm > 1) {
    lv_task_reset(taskPressesToStopAlarmTimeout);
    infiniSleepController.pushesLeftToStopWakeAlarm--;
    UpdateDisplay();
    return true;
  }

  if (infiniSleepController.isSnoozing) {
    infiniSleepController.RestorePreSnoozeTime();
  }
  infiniSleepController.isSnoozing = false;
  StopAlerting();
  if (infiniSleepController.IsTrackerEnabled()) {
    displayState = SleepDisplayState::Info;
    UpdateDisplay();
    infiniSleepController.ToggleTracker();
    UpdateDisplay();
    return true;
  }
  displayState = SleepDisplayState::Info;
  UpdateDisplay();
  return true;
}

bool Sleep::OnTouchEvent(Pinetime::Applications::TouchEvents event) {

  // Swiping should be ignored when in alerting state
  if (infiniSleepController.IsAlerting() && (event == TouchEvents::SwipeDown || event == TouchEvents::SwipeUp ||
                                             event == TouchEvents::SwipeLeft || event == TouchEvents::SwipeRight)) {
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

  const uint16_t totalAlarmMinutes = infiniSleepController.GetCurrentHour() * 60 + infiniSleepController.GetCurrentMinute();
  const uint16_t newSnoozeMinutes = totalAlarmMinutes + SNOOZE_MINUTES;

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
  if (!infiniSleepController.infiniSleepSettings.naturalWake) {
    taskSnoozeWakeAlarm = lv_task_create(SnoozeAlarmTaskCallback, 120 * 1000, LV_TASK_PRIO_MID, this);
  }
  if (infiniSleepController.infiniSleepSettings.naturalWake) {
    motorController.StartNaturalWakeAlarm();
  } else {
    motorController.StartWakeAlarm();
  }
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
  if (infiniSleepController.infiniSleepSettings.naturalWake) {
    motorController.StopNaturalWakeAlarm();
  } else {
    motorController.StopWakeAlarm();
  }
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
  if (displayState == SleepDisplayState::Alarm && infiniSleepController.GetWakeAlarm().isEnabled) {
    lv_switch_on(enableSwitch, anim);
  } else {
    lv_switch_off(enableSwitch, anim);
  }
}