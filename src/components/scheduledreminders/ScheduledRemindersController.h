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
#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>
#include <array>
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    enum class ReminderType : uint8_t {
      Daily = 0,
      Monthly = 1,
      Weekly = 2
    };

    class ScheduledRemindersController {
    public:
      ScheduledRemindersController(Controllers::DateTime& dateTimeController, Controllers::FS& fs);

      void Init(System::SystemTask* systemTask);
      void SaveReminders();
      void ScheduleReminder(uint8_t reminderIndex);
      void DisableReminder(uint8_t reminderIndex);
      void SetOffReminderNow(uint8_t reminderIndex);
      void StopAlerting();
      
      // Getters for individual reminders
      uint8_t GetReminderHours(uint8_t reminderIndex) const;
      uint8_t GetReminderMinutes(uint8_t reminderIndex) const;
      bool IsReminderEnabled(uint8_t reminderIndex) const;
      bool IsReminderAlerting(uint8_t reminderIndex) const;
      const char* GetReminderName(uint8_t reminderIndex) const;
      const char* GetCurrentDynamicText(uint8_t reminderIndex) const;
      void AdvanceDynamicText(uint8_t reminderIndex);
      ReminderType GetReminderType(uint8_t reminderIndex) const;
      uint8_t GetReminderDayOfMonth(uint8_t reminderIndex) const;
      
      // Control individual reminders
      void EnableReminder(uint8_t reminderIndex);
      void ToggleReminder(uint8_t reminderIndex);
      void SetReminderType(uint8_t reminderIndex, ReminderType type);
      void SetReminderDayOfMonth(uint8_t reminderIndex, uint8_t dayOfMonth);
      
      // Control all reminders
      void EnableAllReminders();
      void DisableAllReminders();
      void ToggleAllReminders();
      bool AreAllRemindersEnabled() const;
      
      // Debug/test functions
      void TestReminder(uint8_t reminderIndex);
      void ForceRescheduleAll();
      
      // Get total count of reminders
      static constexpr uint8_t GetReminderCount() { return reminderCount; }

    private:
      static constexpr uint8_t reminderCount = 6;
      static constexpr uint8_t dynamicTextCount = 4;
      static constexpr uint8_t weeklyTextCount = 3;

      struct ReminderSettings {
        uint8_t hours;
        uint8_t minutes;
        bool isEnabled = false;
        ReminderType type = ReminderType::Daily;
        uint8_t dayOfMonth = 1; // For monthly reminders (1-31)
        uint8_t dayOfWeek = 0; // For weekly reminders (0=Sunday, 1=Monday, ..., 6=Saturday)
        char name[16]; // Fixed size for reminder name
        bool isDynamicText = false; // If true, text rotates from a list
        uint8_t textIndex = 0; // Index into the dynamic text list
      };

      struct ReminderData {
        bool isAlerting = false;
        TimerHandle_t timer;
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> reminderTime;
        struct {
          Pinetime::Controllers::ScheduledRemindersController* controller;
          uint8_t reminderIndex;
        } timerData;
      };

      bool remindersChanged = false;
      Controllers::DateTime& dateTimeController;
      Controllers::FS& fs;
      System::SystemTask* systemTask = nullptr;
      
      // Dynamic text options for daily rotating reminders
      std::array<const char*, dynamicTextCount> dynamicTexts = {{
        "Take a break",
        "Stretch your legs", 
        "Drink some water",
        "Check your posture"
      }};

      // Dynamic text options for weekly rotating reminders
      std::array<const char*, weeklyTextCount> weeklyTexts = {{
        "Plan your week",
        "Review goals",
        "Relax and recharge"
      }};

      // Hardcoded default reminders
      std::array<ReminderSettings, reminderCount> reminders = {{
        {8, 0, true, ReminderType::Daily, 1, 0, "Morning", false, 0},      // 8:00 AM Daily
        {12, 0, true, ReminderType::Daily, 1, 0, "Lunch", false, 0},       // 12:00 PM Daily
        {17, 0, true, ReminderType::Daily, 1, 0, "Wellness", true, 0},     // 5:00 PM Daily with dynamic text
        {18, 0, true, ReminderType::Monthly, 1, 0, "Rent Due", false, 0},  // 6:00 PM Monthly on 1st
        {22, 0, true, ReminderType::Daily, 1, 0, "Bedtime", false, 0},     // 10:00 PM Daily
        {19, 0, true, ReminderType::Weekly, 1, 6, "Weekend", true, 0}      // 7:00 PM Saturday with weekly dynamic text
      }};
      
      std::array<ReminderData, reminderCount> reminderData;

      void LoadSettingsFromFile();
      void SaveSettingsToFile() const;
      void ScheduleReminderInternal(uint8_t reminderIndex);
    };
  }
}
