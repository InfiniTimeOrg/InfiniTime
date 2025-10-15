/*  Copyright (C) 2024 InfiniTime Contributors

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
#include "components/scheduledreminders/ScheduledRemindersController.h"
#include "systemtask/SystemTask.h"
#include "task.h"
#include <chrono>
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Controllers;
using namespace std::chrono_literals;

ScheduledRemindersController::ScheduledRemindersController(Controllers::DateTime& dateTimeController, Controllers::FS& fs)
  : dateTimeController {dateTimeController}, fs {fs} {
}

namespace {
  void SetOffReminder(TimerHandle_t xTimer) {
    auto* controller = static_cast<Pinetime::Controllers::ScheduledRemindersController*>(pvTimerGetTimerID(xTimer));
    NRF_LOG_INFO("[ScheduledRemindersController] Timer callback triggered");
    
    // Get the reminder that was scheduled to go off
    uint8_t scheduledReminder = controller->GetCurrentlyScheduledReminder();
    if (scheduledReminder < 7) {
      NRF_LOG_INFO("[ScheduledRemindersController] Triggering reminder %d", scheduledReminder);
      controller->SetOffReminderNow(scheduledReminder);
    }
    
    // Reschedule timer for the next reminder
    // controller->RescheduleTimer();
  }
}

void ScheduledRemindersController::Init(System::SystemTask* systemTask) {
  this->systemTask = systemTask;
  
  // Initialize reminder data
  for (uint8_t i = 0; i < reminderCount; i++) {
    reminderData[i].controller = this;
    reminderData[i].reminderIndex = i;
    reminderData[i].isAlerting = false;
  }
  
  // Create single timer for all reminders
  reminderTimer = xTimerCreate(
    "ReminderTimer",
    1, 
    pdFALSE, 
    this, 
    SetOffReminder);

  LoadSettingsFromFile();
  
  // Schedule the next reminder
  ScheduleNextReminder();
}

void ScheduledRemindersController::SaveReminders() {
  if (remindersChanged) {
    SaveSettingsToFile();
  }
  remindersChanged = false;
}

void ScheduledRemindersController::ScheduleReminder(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount) {
    return;
  }
  
  if (!reminders[reminderIndex].isEnabled) {
    reminders[reminderIndex].isEnabled = true;
    remindersChanged = true;
  }
  
  ScheduleReminderInternal(reminderIndex);
}

void Pinetime::Controllers::ScheduledRemindersController::ScheduleReminderInternal(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount) {
    return;
  }

  auto& settings = reminders[reminderIndex];

  // Only schedule enabled reminders
  if (!settings.isEnabled) {
    return;
  }

  // Mark that reminders have changed and reschedule the timer
  remindersChanged = true;
  ScheduleNextReminder();
  
  NRF_LOG_INFO("[ScheduledRemindersController] Rescheduled timer for reminder %d", reminderIndex);
}


void ScheduledRemindersController::SetOffReminderNow(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount) {
    NRF_LOG_ERROR("[ScheduledRemindersController] Invalid reminder index %d", reminderIndex);
    return;
  }
  
  NRF_LOG_INFO("[ScheduledRemindersController] Setting off reminder %d: %s", reminderIndex, reminders[reminderIndex].name);
  
  // Advance dynamic text for this reminder if it has dynamic text
  if (reminders[reminderIndex].isDynamicText) {
    AdvanceDynamicText(reminderIndex);
  }
  
  reminderData[reminderIndex].isAlerting = true;
  systemTask->PushMessage(System::Messages::SetOffScheduledReminder);
  NRF_LOG_INFO("[ScheduledRemindersController] Pushed SetOffScheduledReminder message to system task");
}

// void ScheduledRemindersController::StopAlerting() {
//   // Stop alerting for all reminders and schedule next instance (like AlarmController)
//   NRF_LOG_INFO("[ScheduledRemindersController] StopAlerting called");
//   for (uint8_t i = 0; i < reminderCount; i++) {
//     if (reminderData[i].isAlerting) {
//       NRF_LOG_INFO("[ScheduledRemindersController] Stopping alerting for reminder %d", i);
//       reminderData[i].isAlerting = false;
//       // Schedule next instance (all reminders are recurring by nature)
//       ScheduleReminderInternal(i);
//     }
//   }
// }

void ScheduledRemindersController::StopAlertingForReminder(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount) {
    NRF_LOG_ERROR("[ScheduledRemindersController] Invalid reminder index %d for StopAlertingForReminder", reminderIndex);
    return;
  }
  
  if (reminderData[reminderIndex].isAlerting) {
    NRF_LOG_INFO("[ScheduledRemindersController] Stopping alerting for specific reminder %d", reminderIndex);
    reminderData[reminderIndex].isAlerting = false;
    ScheduleNextReminder();
  } else {
    NRF_LOG_WARNING("[ScheduledRemindersController] Reminder %d was not alerting when StopAlertingForReminder was called", reminderIndex);
  }
}

uint8_t ScheduledRemindersController::GetReminderHours(uint8_t reminderIndex) const {
  if (reminderIndex >= reminderCount) {
    return 0;
  }
  return reminders[reminderIndex].hours;
}

uint8_t ScheduledRemindersController::GetReminderMinutes(uint8_t reminderIndex) const {
  if (reminderIndex >= reminderCount) {
    return 0;
  }
  return reminders[reminderIndex].minutes;
}

bool ScheduledRemindersController::IsReminderEnabled(uint8_t reminderIndex) const {
  if (reminderIndex >= reminderCount) {
    return false;
  }
  return reminders[reminderIndex].isEnabled;
}

bool ScheduledRemindersController::IsReminderAlerting(uint8_t reminderIndex) const {
  if (reminderIndex >= reminderCount) {
    return false;
  }
  return reminderData[reminderIndex].isAlerting;
}

const char* ScheduledRemindersController::GetReminderName(uint8_t reminderIndex) const {
  if (reminderIndex >= reminderCount) {
    return "";
  }
  
  // If this reminder has dynamic text, return the current dynamic text
  if (reminders[reminderIndex].isDynamicText) {
    return GetCurrentDynamicText(reminderIndex);
  }
  
  return reminders[reminderIndex].name;
}

const char* ScheduledRemindersController::GetCurrentDynamicText(uint8_t reminderIndex) const {
  if (reminderIndex >= reminderCount || !reminders[reminderIndex].isDynamicText) {
    return "";
  }
  
  uint8_t textIndex = reminders[reminderIndex].textIndex;
  
  // Choose the appropriate text array based on reminder type
  if (reminders[reminderIndex].type == Pinetime::Controllers::ReminderType::Weekly) {
    if (textIndex >= weeklyTextCount) {
      textIndex = 0; // Safety fallback
    }
    return weeklyTexts[textIndex];
  } else {
    if (textIndex >= dailyTextCount) {
      textIndex = 0; // Safety fallback
    }
    return dailyTexts[textIndex];
  }
}

void ScheduledRemindersController::AdvanceDynamicText(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount || !reminders[reminderIndex].isDynamicText) {
    return;
  }
  
  // Choose the appropriate text count based on reminder type
  uint8_t maxTextCount = (reminders[reminderIndex].type == Pinetime::Controllers::ReminderType::Weekly) 
                         ? weeklyTextCount 
                         : dailyTextCount;
  
  reminders[reminderIndex].textIndex = (reminders[reminderIndex].textIndex + 1) % maxTextCount;
  remindersChanged = true;
}

void ScheduledRemindersController::EnableAllReminders() {
  for (uint8_t i = 0; i < reminderCount; i++) {
    if (!reminders[i].isEnabled) {
      reminders[i].isEnabled = true;
      ScheduleReminderInternal(i);
    }
  }
  remindersChanged = true;
}

void ScheduledRemindersController::DisableAllReminders() {
  for (uint8_t i = 0; i < reminderCount; i++) {
    if (reminders[i].isEnabled) {
      reminders[i].isEnabled = false;
    }
  }
  
  // Stop the timer since no reminders are enabled
  xTimerStop(reminderTimer, 0);
  
  remindersChanged = true;
}

void ScheduledRemindersController::ToggleAllReminders() {
  if (AreAllRemindersEnabled()) {
    DisableAllReminders();
  } else {
    EnableAllReminders();
  }
}

bool ScheduledRemindersController::AreAllRemindersEnabled() const {
  for (uint8_t i = 0; i < reminderCount; i++) {
    if (!reminders[i].isEnabled) {
      return false;
    }
  }
  return true;
}

void ScheduledRemindersController::LoadSettingsFromFile() {
  lfs_file_t reminderFile;
  std::array<ReminderSettings, reminderCount> reminderBuffer;

  if (fs.FileOpen(&reminderFile, "/.system/reminders.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[ScheduledRemindersController] Failed to open reminders data file, using defaults");
    return;
  }

  fs.FileRead(&reminderFile, reinterpret_cast<uint8_t*>(reminderBuffer.data()), sizeof(reminderBuffer));
  fs.FileClose(&reminderFile);
  
  reminders = reminderBuffer;
  NRF_LOG_INFO("[ScheduledRemindersController] Loaded reminder settings from file");
}

void ScheduledRemindersController::SaveSettingsToFile() const {
  lfs_dir systemDir;
  if (fs.DirOpen("/.system", &systemDir) != LFS_ERR_OK) {
    fs.DirCreate("/.system");
  }
  fs.DirClose(&systemDir);
  
  lfs_file_t reminderFile;
  if (fs.FileOpen(&reminderFile, "/.system/reminders.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[ScheduledRemindersController] Failed to open reminders data file for saving");
    return;
  }

  fs.FileWrite(&reminderFile, reinterpret_cast<const uint8_t*>(reminders.data()), sizeof(reminders));
  fs.FileClose(&reminderFile);
  NRF_LOG_INFO("[ScheduledRemindersController] Saved reminder settings to file");
}

uint8_t ScheduledRemindersController::FindNextReminder() const {
  auto now = dateTimeController.CurrentDateTime();
  uint8_t nextReminder = 7; // Invalid index
  auto nextTime = std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>::max();
  
  for (uint8_t i = 0; i < reminderCount; i++) {
    if (!reminders[i].isEnabled || reminderData[i].isAlerting) {
      continue;
    }
    
    // Calculate next occurrence for this reminder
    auto reminderTime = CalculateNextReminderTime(i, now);
    if (reminderTime < nextTime) {
      nextTime = reminderTime;
      nextReminder = i;
    }
  }
  
  return nextReminder;
}

void ScheduledRemindersController::ScheduleNextReminder() {
  // Stop current timer
  xTimerStop(reminderTimer, 0);
  
  uint8_t nextReminder = FindNextReminder();
  if (nextReminder >= reminderCount) {
    NRF_LOG_INFO("[ScheduledRemindersController] No enabled reminders to schedule");
    currentlyScheduledReminder = 7; // Invalid index
    return;
  }
  
  // Track which reminder is currently scheduled
  currentlyScheduledReminder = nextReminder;
  
  // Calculate delay to next reminder
  auto now = dateTimeController.CurrentDateTime();
  auto nextTime = CalculateNextReminderTime(nextReminder, now);
  
  // Store the calculated time for this reminder
  reminderData[nextReminder].reminderTime = nextTime;
  
  // Calculate delay in seconds
  auto secondsToReminder = std::chrono::duration_cast<std::chrono::seconds>(nextTime - now).count();
  if (secondsToReminder < 1) {
    secondsToReminder = 1;
  }
  
  // Set timer period and start
  xTimerChangePeriod(reminderTimer, secondsToReminder * configTICK_RATE_HZ, 0);
  NRF_LOG_INFO("[ScheduledRemindersController] Timer period set to %ld ticks", (long)(secondsToReminder * configTICK_RATE_HZ));
  xTimerStart(reminderTimer, 0);
  
  NRF_LOG_INFO("[ScheduledRemindersController] Scheduled next reminder %d in %ld seconds", nextReminder, (long)secondsToReminder);
}

void ScheduledRemindersController::RescheduleTimer() {
  ScheduleNextReminder();
}

uint8_t ScheduledRemindersController::GetCurrentlyScheduledReminder() const {
  return currentlyScheduledReminder;
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> 
ScheduledRemindersController::CalculateNextReminderTime(uint8_t reminderIndex, 
  const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& now) const {
  
  if (reminderIndex >= reminderCount) {
    return std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>::max();
  }
  
  const auto& settings = reminders[reminderIndex];
  
  // Convert now to time_t for easier manipulation
  std::time_t ttNow = std::chrono::system_clock::to_time_t(
      std::chrono::time_point_cast<std::chrono::system_clock::duration>(now));
  std::tm* tmTarget = std::localtime(&ttNow);

  // Initialize to current date/time, then set target H:M:S
  tmTarget->tm_sec  = 0;
  tmTarget->tm_min  = settings.minutes;
  tmTarget->tm_hour = settings.hours;

  // Decide next fire date based on recurrence
  switch (settings.type) {
    case ReminderType::Daily: {
      // If target time today has passed, move to tomorrow
      bool passedToday =
          (settings.hours < dateTimeController.Hours()) ||
          (settings.hours == dateTimeController.Hours() && settings.minutes <= dateTimeController.Minutes());
      if (passedToday) {
        tmTarget->tm_mday += 1;
        tmTarget->tm_wday = (tmTarget->tm_wday + 1) % 7;
      }
      break;
    }

    case ReminderType::Weekly: {
      // dayOfWeek: 0=Sun..6=Sat, matches std::tm::tm_wday
      int todayW  = tmTarget->tm_wday;             // 0..6
      int targetW = static_cast<int>(settings.dayOfWeek) % 7;
      int delta   = (targetW - todayW + 7) % 7;

      // If same day but time already passed, push a full week
      bool timePassedToday =
          (delta == 0) &&
          ((settings.hours < dateTimeController.Hours()) ||
           (settings.hours == dateTimeController.Hours() && settings.minutes <= dateTimeController.Minutes()));
      if (timePassedToday) {
        delta = 7;
      } else if (delta == 0) {
        // same day later today → delta stays 0
      }

      tmTarget->tm_mday += delta;
      // tm_wday will be normalized by mktime below
      break;
    }
  }

  // Respect DST automatically
  tmTarget->tm_isdst = -1;

  // Convert back to time_point
  std::time_t ttTarget = std::mktime(tmTarget);
  return std::chrono::system_clock::from_time_t(ttTarget);
}