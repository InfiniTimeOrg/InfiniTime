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
  struct TimerData {
    Pinetime::Controllers::ScheduledRemindersController* controller;
    uint8_t reminderIndex;
  };
  
  void SetOffReminder(TimerHandle_t xTimer) {
    auto* timerData = static_cast<TimerData*>(pvTimerGetTimerID(xTimer));
    NRF_LOG_INFO("[ScheduledRemindersController] Timer callback triggered for reminder %d", timerData->reminderIndex);
    timerData->controller->SetOffReminderNow(timerData->reminderIndex);
  }
}

void ScheduledRemindersController::Init(System::SystemTask* systemTask) {
  this->systemTask = systemTask;
  
  // Create timers for each reminder with timer data
  for (uint8_t i = 0; i < reminderCount; i++) {
    // Set up timer data
    reminderData[i].timerData.controller = this;
    reminderData[i].timerData.reminderIndex = i;
    
    reminderData[i].timer = xTimerCreate("Reminder", 1, pdFALSE, &reminderData[i].timerData, SetOffReminder);
    NRF_LOG_INFO("[ScheduledRemindersController] Created timer for reminder %d", i);
  }
  
  LoadSettingsFromFile();
  
  // Schedule any enabled reminders
  for (uint8_t i = 0; i < reminderCount; i++) {
    if (reminders[i].isEnabled) {
      NRF_LOG_INFO("[ScheduledRemindersController] Loaded reminder %d was enabled, scheduling", i);
      ScheduleReminderInternal(i);
    }
  }
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

void ScheduledRemindersController::ScheduleReminderInternal(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount || !reminders[reminderIndex].isEnabled) {
    NRF_LOG_WARNING("[ScheduledRemindersController] Cannot schedule reminder %d: index=%d, enabled=%d", 
                    reminderIndex, reminderIndex >= reminderCount ? 0 : 1, reminders[reminderIndex].isEnabled);
    return;
  }
  
  NRF_LOG_INFO("[ScheduledRemindersController] Scheduling reminder %d (%s) at %02d:%02d", 
               reminderIndex, reminders[reminderIndex].name, 
               reminders[reminderIndex].hours, reminders[reminderIndex].minutes);
  
  // Stop existing timer
  xTimerStop(reminderData[reminderIndex].timer, 0);

  auto now = dateTimeController.CurrentDateTime();
  reminderData[reminderIndex].reminderTime = now;
  time_t ttReminderTime = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(reminderData[reminderIndex].reminderTime));
  tm* tmReminderTime = std::localtime(&ttReminderTime);
  
  // Debug current time
  NRF_LOG_INFO("[ScheduledRemindersController] Current time: %02d:%02d:%02d", 
               dateTimeController.Hours(), dateTimeController.Minutes(), dateTimeController.Seconds());

  // Use the same logic as AlarmController for daily reminders
  if (reminders[reminderIndex].type == Pinetime::Controllers::ReminderType::Daily) {
    // If the time being set has already passed today, the reminder should be set for tomorrow
    if (reminders[reminderIndex].hours < dateTimeController.Hours() ||
        (reminders[reminderIndex].hours == dateTimeController.Hours() && reminders[reminderIndex].minutes <= dateTimeController.Minutes())) {
      tmReminderTime->tm_mday += 1;
      // tm_wday doesn't update automatically
      tmReminderTime->tm_wday = (tmReminderTime->tm_wday + 1) % 7;
    }
  } else if (reminders[reminderIndex].type == Pinetime::Controllers::ReminderType::Monthly) {
    // Monthly reminder logic
    uint8_t targetDay = reminders[reminderIndex].dayOfMonth;
    uint8_t currentDay = dateTimeController.Day();
    
    // Set the target day of month
    tmReminderTime->tm_mday = targetDay;
    
    // If the target day has already passed this month, set for next month
    if (targetDay < currentDay ||
        (targetDay == currentDay && 
         (reminders[reminderIndex].hours < dateTimeController.Hours() ||
          (reminders[reminderIndex].hours == dateTimeController.Hours() && reminders[reminderIndex].minutes <= dateTimeController.Minutes())))) {
      tmReminderTime->tm_mon += 1;
      // Handle year rollover
      if (tmReminderTime->tm_mon > 11) {
        tmReminderTime->tm_mon = 0;
        tmReminderTime->tm_year += 1;
      }
    }
  } else if (reminders[reminderIndex].type == Pinetime::Controllers::ReminderType::Weekly) {
    // Weekly reminder logic
    uint8_t targetDayOfWeek = reminders[reminderIndex].dayOfWeek;
    // Convert Days enum to uint8_t (Days::Sunday=7, Monday=1, ..., Saturday=6)
    uint8_t currentDayOfWeek = static_cast<uint8_t>(dateTimeController.DayOfWeek());
    if (currentDayOfWeek == 7) currentDayOfWeek = 0; // Convert Sunday from 7 to 0
    
    // Calculate days until target day of week
    int8_t daysUntilTarget = (targetDayOfWeek - currentDayOfWeek + 7) % 7;
    
    // If it's the target day but time has passed, or if it's not the target day
    if (daysUntilTarget == 0 && 
        (reminders[reminderIndex].hours < dateTimeController.Hours() ||
         (reminders[reminderIndex].hours == dateTimeController.Hours() && reminders[reminderIndex].minutes <= dateTimeController.Minutes()))) {
      daysUntilTarget = 7; // Next week
    } else if (daysUntilTarget == 0) {
      daysUntilTarget = 0; // Today, time hasn't passed yet
    }
    
    // Add the days to get to the target day
    tmReminderTime->tm_mday += daysUntilTarget;
    
    // Update day of week (tm_wday doesn't update automatically)
    tmReminderTime->tm_wday = (tmReminderTime->tm_wday + daysUntilTarget) % 7;
  }

  tmReminderTime->tm_hour = reminders[reminderIndex].hours;
  tmReminderTime->tm_min = reminders[reminderIndex].minutes;
  tmReminderTime->tm_sec = 0;
  tmReminderTime->tm_isdst = -1; // use system timezone setting to determine DST

  // Convert back to a time_point
  reminderData[reminderIndex].reminderTime = std::chrono::system_clock::from_time_t(std::mktime(tmReminderTime));
  auto secondsToReminder = std::chrono::duration_cast<std::chrono::seconds>(reminderData[reminderIndex].reminderTime - now).count();
  
  // Use the same approach as AlarmController - no minimum check, let FreeRTOS handle it
  NRF_LOG_INFO("[ScheduledRemindersController] Scheduling reminder %d for %ld seconds", reminderIndex, secondsToReminder);
  
  // Use the same timer approach as AlarmController
  xTimerChangePeriod(reminderData[reminderIndex].timer, secondsToReminder * configTICK_RATE_HZ, 0);
  xTimerStart(reminderData[reminderIndex].timer, 0);
  
  NRF_LOG_INFO("[ScheduledRemindersController] Successfully started timer for reminder %d", reminderIndex);
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

void ScheduledRemindersController::StopAlerting() {
  // Stop alerting for all reminders and schedule next instance (like AlarmController)
  for (uint8_t i = 0; i < reminderCount; i++) {
    if (reminderData[i].isAlerting) {
      reminderData[i].isAlerting = false;
      // Schedule next instance (all reminders are recurring by nature)
      ScheduleReminderInternal(i);
    }
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
    if (textIndex >= dynamicTextCount) {
      textIndex = 0; // Safety fallback
    }
    return dynamicTexts[textIndex];
  }
}

void ScheduledRemindersController::AdvanceDynamicText(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount || !reminders[reminderIndex].isDynamicText) {
    return;
  }
  
  // Choose the appropriate text count based on reminder type
  uint8_t maxTextCount = (reminders[reminderIndex].type == Pinetime::Controllers::ReminderType::Weekly) 
                         ? weeklyTextCount 
                         : dynamicTextCount;
  
  reminders[reminderIndex].textIndex = (reminders[reminderIndex].textIndex + 1) % maxTextCount;
  remindersChanged = true;
}

Pinetime::Controllers::ReminderType ScheduledRemindersController::GetReminderType(uint8_t reminderIndex) const {
  if (reminderIndex >= reminderCount) {
    return Pinetime::Controllers::ReminderType::Daily;
  }
  return reminders[reminderIndex].type;
}

uint8_t ScheduledRemindersController::GetReminderDayOfMonth(uint8_t reminderIndex) const {
  if (reminderIndex >= reminderCount) {
    return 1;
  }
  return reminders[reminderIndex].dayOfMonth;
}

void ScheduledRemindersController::EnableReminder(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount) {
    return;
  }
  
  if (!reminders[reminderIndex].isEnabled) {
    reminders[reminderIndex].isEnabled = true;
    remindersChanged = true;
    ScheduleReminderInternal(reminderIndex);
  }
}

void ScheduledRemindersController::ToggleReminder(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount) {
    return;
  }
  
  if (reminders[reminderIndex].isEnabled) {
    xTimerStop(reminderData[reminderIndex].timer, 0);
    reminders[reminderIndex].isEnabled = false;
    remindersChanged = true;
  } else {
    EnableReminder(reminderIndex);
  }
}

void ScheduledRemindersController::SetReminderType(uint8_t reminderIndex, Pinetime::Controllers::ReminderType type) {
  if (reminderIndex >= reminderCount) {
    return;
  }
  
  if (reminders[reminderIndex].type != type) {
    reminders[reminderIndex].type = type;
    remindersChanged = true;
    
    // If reminder is enabled, reschedule it with new type
    if (reminders[reminderIndex].isEnabled) {
      xTimerStop(reminderData[reminderIndex].timer, 0);
      ScheduleReminderInternal(reminderIndex);
    }
  }
}

void ScheduledRemindersController::SetReminderDayOfMonth(uint8_t reminderIndex, uint8_t dayOfMonth) {
  if (reminderIndex >= reminderCount) {
    return;
  }
  
  // Validate day of month (1-31)
  if (dayOfMonth < 1 || dayOfMonth > 31) {
    return;
  }
  
  if (reminders[reminderIndex].dayOfMonth != dayOfMonth) {
    reminders[reminderIndex].dayOfMonth = dayOfMonth;
    remindersChanged = true;
    
    // If reminder is enabled and is monthly, reschedule it
    if (reminders[reminderIndex].isEnabled && reminders[reminderIndex].type == Pinetime::Controllers::ReminderType::Monthly) {
      xTimerStop(reminderData[reminderIndex].timer, 0);
      ScheduleReminderInternal(reminderIndex);
    }
  }
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
      xTimerStop(reminderData[i].timer, 0);
      reminders[i].isEnabled = false;
    }
  }
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

void ScheduledRemindersController::TestReminder(uint8_t reminderIndex) {
  if (reminderIndex >= reminderCount) {
    NRF_LOG_ERROR("[ScheduledRemindersController] Invalid reminder index for test: %d", reminderIndex);
    return;
  }
  
  NRF_LOG_INFO("[ScheduledRemindersController] Testing reminder %d: %s", reminderIndex, reminders[reminderIndex].name);
  SetOffReminderNow(reminderIndex);
}

void ScheduledRemindersController::ForceRescheduleAll() {
  NRF_LOG_INFO("[ScheduledRemindersController] Force rescheduling all reminders");
  for (uint8_t i = 0; i < reminderCount; i++) {
    if (reminders[i].isEnabled) {
      NRF_LOG_INFO("[ScheduledRemindersController] Force rescheduling reminder %d", i);
      ScheduleReminderInternal(i);
    }
  }
}

