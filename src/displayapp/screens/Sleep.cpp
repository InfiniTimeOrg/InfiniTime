/*  Copyright (C) 2025 Asger Gitz-Johansen

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "Sleep.h"

#include <components/sleeptracking/SleepTrackingController.h>
#include <libraries/log/nrf_log.h>

namespace {
  void OnValueChangedHandler(void* userData) {
    static_cast<Pinetime::Applications::Screens::Sleep*>(userData)->OnAlarmValueChanged();
  }

  void OnButtonPress(lv_obj_t* obj, lv_event_t event) {
    static_cast<Pinetime::Applications::Screens::Sleep*>(obj->user_data)->OnButtonEvent(obj, event);
  }

  void StopAlarmTaskCallback(lv_task_t* task) {
    static_cast<Pinetime::Applications::Screens::Sleep*>(task->user_data)->StopAlerting();
  }
}

namespace Pinetime::Applications::Screens {
  Sleep::Sleep(Controllers::SleepTrackingController& sleeptrackingController,
               Controllers::Settings::ClockType clockType,
               Controllers::MotorController& motorController,
               System::SystemTask& systemTask,
               DisplayApp& displayApp)
    : sleeptrackingController {sleeptrackingController},
      motorController {motorController},
      systemTask {systemTask},
      displayApp {displayApp},
      wakeLock {systemTask},
      state {},
      hourCounter {0, 23, jetbrains_mono_76},
      minuteCounter {0, 59, jetbrains_mono_76},
      lblampm {nullptr},
      startButton {nullptr},
      startText {nullptr},
      stopButton {nullptr},
      stopText {nullptr},
      dismissButton {nullptr},
      dismissText {nullptr},
      colonLabel {nullptr},
      wakeUpLabel {nullptr},
      sleepingLabel {nullptr},
      taskStopAlarm {nullptr} {
    auto settings = sleeptrackingController.GetSettings();
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
    hourCounter.SetValue(settings.alarm.hours);
    hourCounter.SetValueChangedEventCallback(this, OnValueChangedHandler);

    minuteCounter.Create();
    lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
    minuteCounter.SetValue(settings.alarm.minutes);
    minuteCounter.SetValueChangedEventCallback(this, OnValueChangedHandler);

    lv_obj_t* colonLabel = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_font(colonLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
    lv_label_set_text_static(colonLabel, ":");
    lv_obj_align(colonLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, -29);

    stopButton = lv_btn_create(lv_scr_act(), nullptr);
    stopButton->user_data = this;
    lv_obj_set_event_cb(stopButton, OnButtonPress);
    lv_obj_set_size(stopButton, 240, 50);
    lv_obj_align(stopButton, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_local_bg_color(stopButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    stopText = lv_label_create(stopButton, nullptr);
    lv_label_set_text_static(stopText, Symbols::pause);
    lv_obj_set_hidden(stopButton, true);

    dismissButton = lv_btn_create(lv_scr_act(), nullptr);
    dismissButton->user_data = this;
    lv_obj_set_event_cb(dismissButton, OnButtonPress);
    lv_obj_set_size(dismissButton, 240, 50);
    lv_obj_align(dismissButton, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_local_bg_color(dismissButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    dismissText = lv_label_create(dismissButton, nullptr);
    lv_label_set_text_static(dismissText, Symbols::stop);
    lv_obj_set_hidden(dismissButton, true);

    startButton = lv_btn_create(lv_scr_act(), nullptr);
    startButton->user_data = this;
    lv_obj_set_event_cb(startButton, OnButtonPress);
    lv_obj_set_size(startButton, 240, 50);
    lv_obj_align(startButton, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_local_bg_color(startButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    startText = lv_label_create(startButton, nullptr);
    lv_label_set_text_static(startText, Symbols::bed);
    lv_obj_set_hidden(startButton, false);

    UpdateWakeAlarmTime();

    if (sleeptrackingController.IsAlerting()) {
      StartAlerting();
    } else if (sleeptrackingController.IsTracking()) {
      SetState(State::Tracking);
    } else {
      SetState(State::NotTracking);
    }
  }

  Sleep::~Sleep() {
    if (sleeptrackingController.IsAlerting()) {
      StopAlerting();
    }
    lv_obj_clean(lv_scr_act());
    sleeptrackingController.SaveSettings();
  }

  void Sleep::StartAlerting() {
    SetState(State::Alerting);
    sleeptrackingController.OnGentleWakeupTimerTrigger();
  }

  void Sleep::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
    if (event != LV_EVENT_CLICKED) {
      return;
    }
    if (obj == stopButton) {
      sleeptrackingController.StopTracking();
      SetState(State::NotTracking);
      return;
    }
    if (obj == dismissButton) {
      sleeptrackingController.StopTracking();
      SetState(State::NotTracking);
      return;
    }
    if (obj == startButton) {
      sleeptrackingController.StartTracking();
      SetState(State::Tracking);
      return;
    }
  }

  void Sleep::StopAlerting() {
    sleeptrackingController.StopTracking();
    SetState(State::NotTracking);
  }

  void Sleep::OnAlarmValueChanged() {
    UpdateWakeAlarmTime();
  }

  void Sleep::UpdateWakeAlarmTime() {
    if (lblampm != nullptr) {
      if (hourCounter.GetValue() >= 12) {
        lv_label_set_text_static(lblampm, "PM");
      } else {
        lv_label_set_text_static(lblampm, "AM");
      }
    }
    auto settings = sleeptrackingController.GetSettings();
    settings.alarm.hours = hourCounter.GetValue();
    settings.alarm.minutes = minuteCounter.GetValue();
    sleeptrackingController.SetSettings(settings);
  }

  bool Sleep::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
    // Don't allow closing the screen by swiping while the wake alarm is alerting
    return sleeptrackingController.IsAlerting() && event == TouchEvents::SwipeDown;
  }

  void Sleep::SetState(const State& uistate) {
    state = uistate;
    switch (uistate) {
      case State::NotTracking:
        wakeLock.Release();
        lv_obj_set_hidden(stopButton, true);
        lv_obj_set_hidden(dismissButton, true);
        hourCounter.ShowControls();
        minuteCounter.ShowControls();
        lv_obj_set_hidden(startButton, false);
        if (taskStopAlarm != nullptr) {
          lv_task_del(taskStopAlarm);
          taskStopAlarm = nullptr;
        }
        break;
      case State::Tracking:
        wakeLock.Release();
        lv_obj_set_hidden(startButton, true);
        lv_obj_set_hidden(dismissButton, true);
        hourCounter.HideControls();
        minuteCounter.HideControls();
        lv_obj_set_hidden(stopButton, false);
        break;
      case State::Alerting:
        wakeLock.Lock();
        lv_obj_set_hidden(startButton, true);
        lv_obj_set_hidden(stopButton, true);
        hourCounter.HideControls();
        minuteCounter.HideControls();
        lv_obj_set_hidden(dismissButton, false);
        if (taskStopAlarm != nullptr) {
          lv_task_del(taskStopAlarm);
          taskStopAlarm = nullptr;
        }
        taskStopAlarm = lv_task_create(StopAlarmTaskCallback, pdMS_TO_TICKS(10 * 60 * 1000), LV_TASK_PRIO_MID, this);
        break;
    }
  }

  bool Sleep::OnButtonPushed() {
    if (sleeptrackingController.IsAlerting()) {
      sleeptrackingController.StopTracking();
      SetState(State::NotTracking);
      return true;
    }
    return false;
  }
}
