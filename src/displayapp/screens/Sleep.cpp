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
  screen->StopAlerting();
  screen->SnoozeWakeAlarm();
}

Sleep::Sleep(Controllers::InfiniSleepController& infiniSleepController,
             Controllers::Settings::ClockType clockType,
             System::SystemTask& systemTask,
             Controllers::MotorController& motorController)
  : infiniSleepController {infiniSleepController}, wakeLock(systemTask), motorController {motorController}, clockType {clockType} {

  UpdateDisplay();
  taskRefresh = lv_task_create(RefreshTaskCallback, 2000, LV_TASK_PRIO_MID, this);
}

Sleep::~Sleep() {
  if (infiniSleepController.IsAlerting()) {
    StopAlerting();
  }
  lv_task_del(taskRefresh);
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
      break;
    case SleepDisplayState::Info:
      DrawInfoScreen();
      break;
    case SleepDisplayState::Settings:
      DrawSettingsScreen();
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
  hourCounter.SetValue(infiniSleepController.Hours());
  hourCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  minuteCounter.Create();
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  minuteCounter.SetValue(infiniSleepController.Minutes());
  minuteCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  lv_obj_t* colonLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(colonLabel, ":");
  lv_obj_align(colonLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -29);

  btnStop = lv_btn_create(lv_scr_act(), nullptr);
  btnStop->user_data = this;
  lv_obj_set_event_cb(btnStop, btnEventHandler);
  lv_obj_set_size(btnStop, 115, 50);
  lv_obj_align(btnStop, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnStop, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  txtStop = lv_label_create(btnStop, nullptr);
  lv_label_set_text_static(txtStop, Symbols::stop);
  lv_obj_set_hidden(btnStop, true);

  static constexpr lv_color_t bgColor = Colors::bgAlt;

  btnSuggestedAlarm = lv_btn_create(lv_scr_act(), nullptr);
  btnSuggestedAlarm->user_data = this;
  lv_obj_set_event_cb(btnSuggestedAlarm, btnEventHandler);
  lv_obj_set_size(btnSuggestedAlarm, 115, 50);
  lv_obj_align(btnSuggestedAlarm, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  txtSuggestedAlarm = lv_label_create(btnSuggestedAlarm, nullptr);
  lv_label_set_text_static(txtSuggestedAlarm, "Suggested");

  // btnRecur = lv_btn_create(lv_scr_act(), nullptr);
  // btnRecur->user_data = this;
  // lv_obj_set_event_cb(btnRecur, btnEventHandler);
  // lv_obj_set_size(btnRecur, 115, 50);
  // lv_obj_align(btnRecur, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  // txtRecur = lv_label_create(btnRecur, nullptr);
  // SetRecurButtonState();
  // lv_obj_set_style_local_bg_color(btnRecur, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, bgColor);

  btnInfo = lv_btn_create(lv_scr_act(), nullptr);
  btnInfo->user_data = this;
  lv_obj_set_event_cb(btnInfo, btnEventHandler);
  lv_obj_set_size(btnInfo, 50, 50);
  lv_obj_align(btnInfo, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, -4);
  lv_obj_set_style_local_bg_color(btnInfo, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, bgColor);
  lv_obj_set_style_local_border_width(btnInfo, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_border_color(btnInfo, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  lv_obj_t* txtInfo = lv_label_create(btnInfo, nullptr);
  lv_label_set_text_static(txtInfo, "i");

  enableSwitch = lv_switch_create(lv_scr_act(), nullptr);
  enableSwitch->user_data = this;
  lv_obj_set_event_cb(enableSwitch, btnEventHandler);
  lv_obj_set_size(enableSwitch, 100, 50);
  // Align to the center of 115px from edge
  lv_obj_align(enableSwitch, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 7, 0);
  lv_obj_set_style_local_bg_color(enableSwitch, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, bgColor);

  UpdateWakeAlarmTime();

  if (infiniSleepController.IsAlerting()) {
    SetAlerting();
  } else {
    SetSwitchState(LV_ANIM_OFF);
  }
}

void Sleep::DrawInfoScreen() {
  lv_obj_t* lblTime = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(lblTime, "%02d:%02d", infiniSleepController.GetCurrentHour(), infiniSleepController.GetCurrentMinute());
  lv_obj_align(lblTime, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 5);
  lv_obj_set_style_local_text_color(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  label_hr = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(label_hr, "HR: %d", infiniSleepController.rollingBpm);
  lv_obj_align(label_hr, lblTime, LV_ALIGN_CENTER, 0, 50);
  lv_obj_set_style_local_text_color(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, infiniSleepController.IsEnabled() ? LV_COLOR_RED : LV_COLOR_WHITE);
  
  if (infiniSleepController.IsEnabled()) {
    label_start_time = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_fmt(label_start_time, "Began at: %02d:%02d", infiniSleepController.startTimeHours, infiniSleepController.startTimeMinutes);
    lv_obj_align(label_start_time, label_hr, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_local_text_color(label_start_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  }

  // The alarm info
  label_alarm_time = lv_label_create(lv_scr_act(), nullptr);
  if (infiniSleepController.GetWakeAlarm().isEnabled) {
    lv_label_set_text_fmt(label_alarm_time, "Alarm at: %02d:%02d", infiniSleepController.GetWakeAlarm().hours, infiniSleepController.GetWakeAlarm().minutes);
  } else {
    lv_label_set_text_static(label_alarm_time, "Alarm is not set.");
  }
  lv_obj_align(label_alarm_time, label_hr, LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_local_text_color(label_alarm_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, infiniSleepController.IsEnabled() ? LV_COLOR_RED : LV_COLOR_WHITE);

  // Gradual Wake info
  label_gradual_wake = lv_label_create(lv_scr_act(), nullptr);
  if (infiniSleepController.GetInfiniSleepSettings().graddualWake) {
    lv_label_set_text_fmt(label_gradual_wake, "Gradual Wake: %d/9", infiniSleepController.gradualWakeVibration);
  } else {
    lv_label_set_text_static(label_gradual_wake, "Gradual Wake: OFF");
  }
  lv_obj_align(label_gradual_wake, label_hr, LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_local_text_color(label_gradual_wake, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, infiniSleepController.IsEnabled() ? LV_COLOR_RED : LV_COLOR_WHITE);

  // Sleep Cycles Info
  label_sleep_cycles = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(label_sleep_cycles, "Sleep Cycles: %d.%02d", infiniSleepController.GetSleepCycles() / 100, infiniSleepController.GetSleepCycles() % 100);
  lv_obj_align(label_sleep_cycles, label_hr, LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_local_text_color(label_sleep_cycles, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, infiniSleepController.IsEnabled() ? LV_COLOR_RED : LV_COLOR_WHITE);

  // Total sleep time
  label_total_sleep = lv_label_create(lv_scr_act(), nullptr);

  uint16_t totalMinutes = infiniSleepController.GetTotalSleep();

  lv_label_set_text_fmt(label_total_sleep, "Total Sleep: %dh%dm", totalMinutes / 60, totalMinutes % 60);
  lv_obj_align(label_total_sleep, label_hr, LV_ALIGN_CENTER, 0, 100);
  lv_obj_set_style_local_text_color(label_total_sleep, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, infiniSleepController.IsEnabled() ? LV_COLOR_RED : LV_COLOR_WHITE);

  trackerToggleBtn = lv_btn_create(lv_scr_act(), nullptr);
  trackerToggleBtn->user_data = this;
  lv_obj_align(trackerToggleBtn, lv_scr_act(), LV_ALIGN_CENTER, 0, 100);

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

  Setting settings[] = {
    {"Body Tracking", infiniSleepController.BodyTrackingEnabled()},
    {"Heart Rate\nTracking", infiniSleepController.HeartRateTrackingEnabled(), 60},
    {"Gradual Wake", infiniSleepController.GradualWakeEnabled()},
    {"Smart Alarm\n(alpha)", infiniSleepController.SmartAlarmEnabled()}
  };

  int y_offset = 50;
  for (const auto& setting : settings) {
    //lv_obj_t* lblSetting = lv_label_create(lv_scr_act(), nullptr);
   // lv_label_set_text_static(lblSetting, setting.name);
   // lv_obj_align(lblSetting, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, y_offset);

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
    //lv_obj_set_event_cb(lblSetting, settingsToggleEventHandler);

    y_offset += setting.offsetAfter; // Increase the offset to provide better spacing
  }
}

void Sleep::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnStop) {
      StopAlerting();
      SnoozeWakeAlarm();
      return;
    }
    if (obj == btnInfo) {
      ShowAlarmInfo();
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

      hourCounter.SetValue(alarmHour);
      minuteCounter.SetValue(alarmMinute);

      infiniSleepController.DisableWakeAlarm();
      UpdateWakeAlarmTime();
      lv_switch_on(enableSwitch, LV_ANIM_OFF);
      infiniSleepController.ScheduleWakeAlarm();
      return;
    }
    if (obj == btnRecur) {
      DisableWakeAlarm();
      ToggleRecurrence();
    }
  }
}

bool Sleep::OnButtonPushed() {
  if (txtMessage != nullptr && btnMessage != nullptr) {
    HideAlarmInfo();
    return true;
  }
  if (infiniSleepController.IsAlerting()) {
    if (infiniSleepController.pushesLeftToStopWakeAlarm > 1) {
      infiniSleepController.pushesLeftToStopWakeAlarm--;
      return true;
    } else {
      infiniSleepController.isSnoozing = false;
      infiniSleepController.RestorePreSnoozeTime();
      StopAlerting();
      UpdateDisplay();
      return true;
    }
  }
  return false;
}

bool Sleep::OnTouchEvent(Pinetime::Applications::TouchEvents event) {

  // Swiping should be ignored when in alerting state
  if (infiniSleepController.IsAlerting() && 
    (event != TouchEvents::SwipeDown && event != TouchEvents::SwipeUp && event != TouchEvents::SwipeLeft && event != TouchEvents::SwipeRight)) {
    return true;
  }

  // The cases for swiping to change page on app
  switch (event) {
    case TouchEvents::SwipeRight:
      if (displayState != SleepDisplayState::Alarm) {
        displayState = static_cast<SleepDisplayState>(static_cast<int>(displayState) - 1);
        UpdateDisplay();
      }
      NRF_LOG_INFO("SwipeLeft: %d", static_cast<int>(displayState));
      return true;
    case TouchEvents::SwipeLeft:
      if (displayState != SleepDisplayState::Settings) {
        displayState = static_cast<SleepDisplayState>(static_cast<int>(displayState) + 1);
        UpdateDisplay();
      }
      NRF_LOG_INFO("SwipeRight: %d", static_cast<int>(displayState));
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
  if (minuteCounter.GetValue() >= 55) {
    minuteCounter.SetValue(0);
    hourCounter.SetValue((infiniSleepController.GetCurrentHour() + 1));
  } else {
    minuteCounter.SetValue(infiniSleepController.GetCurrentMinute() + SNOOZE_MINUTES);
  }
  infiniSleepController.SetPreSnoozeTime();
  infiniSleepController.isSnoozing = true;
  UpdateWakeAlarmTime();
  SetSwitchState(LV_ANIM_OFF);
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
}

void Sleep::SetAlerting() {
  lv_obj_set_hidden(enableSwitch, true);
  lv_obj_set_hidden(btnStop, false);
  taskSnoozeWakeAlarm = lv_task_create(SnoozeAlarmTaskCallback, pdMS_TO_TICKS(180 * 1000), LV_TASK_PRIO_MID, this);
  motorController.StartAlarm();
  wakeLock.Lock();
}

void Sleep::StopAlerting() {
  infiniSleepController.StopAlerting();
  motorController.StopAlarm();
  SetSwitchState(LV_ANIM_OFF);
  if (taskSnoozeWakeAlarm != nullptr) {
    lv_task_del(taskSnoozeWakeAlarm);
    taskSnoozeWakeAlarm = nullptr;
  }
  wakeLock.Release();
  lv_obj_set_hidden(enableSwitch, false);
  lv_obj_set_hidden(btnStop, true);
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

void Sleep::SetRecurButtonState() {
  using Pinetime::Controllers::InfiniSleepController;
  switch (infiniSleepController.Recurrence()) {
    case InfiniSleepController::RecurType::None:
      lv_label_set_text_static(txtRecur, "ONCE");
      break;
    case InfiniSleepController::RecurType::Daily:
      lv_label_set_text_static(txtRecur, "DAILY");
      break;
    case InfiniSleepController::RecurType::Weekdays:
      lv_label_set_text_static(txtRecur, "MON-FRI");
      break;
  }
}

void Sleep::ToggleRecurrence() {
  using Pinetime::Controllers::InfiniSleepController;
  switch (infiniSleepController.Recurrence()) {
    case InfiniSleepController::RecurType::None:
      infiniSleepController.SetRecurrence(InfiniSleepController::RecurType::Daily);
      break;
    case InfiniSleepController::RecurType::Daily:
      infiniSleepController.SetRecurrence(InfiniSleepController::RecurType::Weekdays);
      break;
    case InfiniSleepController::RecurType::Weekdays:
      infiniSleepController.SetRecurrence(InfiniSleepController::RecurType::None);
      break;
  }
  SetRecurButtonState();
}