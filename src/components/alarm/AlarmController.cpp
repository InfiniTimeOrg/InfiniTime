//
// Created by mrussell on 30.08.21.
//
// Copied from Florian's Timer app

#include "AlarmController.h"
#include "systemtask/SystemTask.h"
#include "app_timer.h"
#include "task.h"
#include <chrono>

using namespace Pinetime::Controllers;
using namespace std::chrono_literals;

AlarmController::AlarmController(Controllers::DateTime& dateTimeController) : dateTimeController {dateTimeController} {
}

APP_TIMER_DEF(alarmAppTimer);

namespace {
  void SetOffAlarm(void* p_context) {
    auto* controller = static_cast<Pinetime::Controllers::AlarmController*>(p_context);
    if (controller != nullptr)
      controller->SetOffAlarmNow();
  }
}

void AlarmController::Init() {
  app_timer_create(&alarmAppTimer, APP_TIMER_MODE_SINGLE_SHOT, SetOffAlarm);
}

void AlarmController::SetAlarm(uint8_t alarmHr, uint8_t alarmMin) {
  hours = alarmHr;
  minutes = alarmMin;
  state = AlarmState::Set;
  scheduleAlarm();
}

void AlarmController::scheduleAlarm() {
  // Determine the next time the alarm needs to go off and set the app_timer
  app_timer_stop(alarmAppTimer);

  auto now = dateTimeController.CurrentDateTime();
  alarmTime = now;
  time_t ttAlarmTime = std::chrono::system_clock::to_time_t(alarmTime);
  tm* tmAlarmTime = std::localtime(&ttAlarmTime);

  // If the time being set has already passed today,the alarm should be set for tomorrow
  if (hours < dateTimeController.Hours() || (hours == dateTimeController.Hours() && minutes <= dateTimeController.Minutes())) {
    tmAlarmTime->tm_mday += 1;
    // tm_wday doesn't update automatically
    tmAlarmTime->tm_wday = (tmAlarmTime->tm_wday + 1) % 7;
  }

  tmAlarmTime->tm_hour = hours;
  tmAlarmTime->tm_min = minutes;
  tmAlarmTime->tm_sec = 0;

  // if alarm is in weekday-only mode, make sure it shifts to the next weekday
  if (recurrence == RecurType::Weekdays) {
    if (tmAlarmTime->tm_wday == 0) { // Sunday, shift 1 day
      tmAlarmTime->tm_mday += 1;
    } else if (tmAlarmTime->tm_wday == 6) { // Saturday, shift 2 days
      tmAlarmTime->tm_mday += 2;
    }
  }
  tmAlarmTime->tm_isdst = -1; // use system timezone setting to determine DST

  // now can convert back to a time_point
  alarmTime = std::chrono::system_clock::from_time_t(std::mktime(tmAlarmTime));
  auto mSecToAlarm = std::chrono::duration_cast<std::chrono::milliseconds>(alarmTime - now).count();
  app_timer_start(alarmAppTimer, APP_TIMER_TICKS(mSecToAlarm), this);
}

uint32_t AlarmController::SecondsToAlarm() {
  return std::chrono::duration_cast<std::chrono::seconds>(alarmTime - dateTimeController.CurrentDateTime()).count();
}

void AlarmController::DisableAlarm() {
  app_timer_stop(alarmAppTimer);
  state = AlarmState::Not_Set;
}

void AlarmController::SetOffAlarmNow() {
  state = AlarmState::Alerting;
  if (systemTask != nullptr) {
    systemTask->PushMessage(System::Messages::SetOffAlarm);
  }
}

void AlarmController::StopAlerting() {
  if (systemTask != nullptr) {
    systemTask->PushMessage(System::Messages::StopRinging);
  }

  // Alarm state is off unless this is a recurring alarm
  if (recurrence == RecurType::None) {
    state = AlarmState::Not_Set;
  } else {
    state = AlarmState::Set;
    // set next instance
    scheduleAlarm();
  }
}

void AlarmController::ToggleRecurrence() {
  if (recurrence == AlarmController::RecurType::None) {
    recurrence = AlarmController::RecurType::Daily;
  } else if (recurrence == AlarmController::RecurType::Daily) {
    recurrence = AlarmController::RecurType::Weekdays;
  } else {
    recurrence = AlarmController::RecurType::None;
  }
}

void AlarmController::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}
