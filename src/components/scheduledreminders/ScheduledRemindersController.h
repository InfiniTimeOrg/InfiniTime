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
      void StopAlertingForReminder(uint8_t reminderIndex);
      
      // Getters for individual reminders
      uint8_t GetReminderHours(uint8_t reminderIndex) const;
      uint8_t GetReminderMinutes(uint8_t reminderIndex) const;
      bool IsReminderEnabled(uint8_t reminderIndex) const;
      bool IsReminderAlerting(uint8_t reminderIndex) const;
      const char* GetReminderName(uint8_t reminderIndex) const;
      const char* GetCurrentDynamicText(uint8_t reminderIndex) const;
      void AdvanceDynamicText(uint8_t reminderIndex);
      
      // Control all reminders
      void EnableAllReminders();
      void DisableAllReminders();
      void ToggleAllReminders();
      bool AreAllRemindersEnabled() const;
      
      // Get total count of reminders
      static constexpr uint8_t GetReminderCount() { return reminderCount; }
      
      // Public methods needed for timer callback
      uint8_t FindNextReminder() const;
      void RescheduleTimer();
      uint8_t GetCurrentlyScheduledReminder() const;

    public:
      struct ReminderData {
        bool isAlerting = false;
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> reminderTime;
        Pinetime::Controllers::ScheduledRemindersController* controller;
        uint8_t reminderIndex;
      };

    private:
      static constexpr uint8_t reminderCount = 7;
      static constexpr uint8_t dailyTextCount = 29;
      static constexpr uint8_t weeklyTextCount = 8;
      // std::array<StaticTimer_t, reminderCount> timerStorage;

      struct ReminderSettings {
        uint8_t hours;
        uint8_t minutes;
        bool isEnabled = false;
        ReminderType type = ReminderType::Daily;
        uint8_t dayOfMonth = 1; // For monthly reminders (1-31)
        uint8_t dayOfWeek = 0; // For weekly reminders (0=Sunday, 1=Monday, ..., 6=Saturday)
        char name[256]; // Fixed size for reminder name
        bool isDynamicText = false; // If true, text rotates from a list
        uint8_t textIndex = 0; // Index into the dynamic text list
      };

      bool remindersChanged = false;
      Controllers::DateTime& dateTimeController;
      Controllers::FS& fs;
      System::SystemTask* systemTask = nullptr;
      
      // Single timer for all reminders
      TimerHandle_t reminderTimer;
      uint8_t currentlyScheduledReminder = 7; // Track which reminder is currently scheduled
      
      // Dynamic text options for daily rotating reminders
      std::array<const char*, dailyTextCount> dailyTexts = {{
        "Wear hearing aids for at least 8 hours/day for full benefit",
        "Fill your med prescription regularly",
        "Remain smoke-free with FDA approved therapies at no cost (1-800-QUIT NOW)",
        "No water, hair/bodysprays, cleaners on hearing aids",
        "Do not drop hearing aids",
        "Very hot or windy places? Remove hearing aid",
        "Noisy place? Remove hearing and wear ear plugs",
        "Before MRI/CT scan - remove hearing aids",
        "Clean reusable earplugs with soap and water after each use",
        "Check and replace - Dirty eartube, eartips, waxguards",
        "Remind others to face you and to speak clearly",
        "Be in a well-lit room when talking",
        "Stuck eartip? Try removing with fingers or go to emergency. No sharp objects in ear",
        "Ear pain? Change eartip, try a different size. Use otoscope to check. See ENT",
        "Check your medicine list for drugs that may harm hearing",
        "Wear in workplace - for improved communication, job performance, safety",
        "Fall, head injury? Get prompt medical help to check brain injury, worsening hearing",
        "Doctor visits, hospital stays? Wear hearing aids to not miss any instruction",
        "Be physically active to support brain health",
        "Protect your head with helmet, when needed, for brain health",
        "Support your brain health with daily wear of hearing aids",
        "Use public assistive technology like hearing loops, with hearing aid telecoil",
        "Be prepared for emergencies with special alerting devices",
        "Check how to wear Personal Protective Equipment with hearing aids at work",
        "Use vibrating/visual alerts to be prepared for emergencies",
        "Check your vision and wear eyeglasses along with your hearing aids",
        "Keep mentally active with reading, crafting, volunteering for brain health",
        "Prevent falls by checking home safety, wearing proper shoes - for brain health",
        "Follow a heart healthy diet such as DASH, MIND"
      }};
    

      // Dynamic text options for weekly rotating reminders
      std::array<const char*, weeklyTextCount> weeklyTexts = {{
        "Wear in different places to get used to the new sounds",
        "Clean excess earwax from ear with FDA listed drops",
        "Check for FDA Red Flag conditions and see an ENT",
        "Manage hypertension, diabetes for brain health",
        "Quit smoking for free (1-800-QUIT-NOW)for brain health",
        "Sleep for 6-8 hours daily for brain health",
        "Limit or avoid alcohol for brain health",
        "Stay connected with family and friends for brain health"
      }};

      // Hardcoded default reminders
      std::array<ReminderSettings, reminderCount> reminders = {{
        // Hours, Minutes, Enabled, Type, DayOfMonth, DayOfWeek, Name, DynamicText, TextIndex
        {9, 0, true, ReminderType::Daily, 1, 0, "Are you wearing your hearing aid, taking your daily meds?", false, 0},      // 9:00 AM Daily
        {17, 0, true, ReminderType::Daily, 1, 0, "Still wearing hearing aid? Before bed, take off, wipe all parts and store safely. Take your daily meds", false, 0},       // 5:00 PM Daily
        {17, 45, true, ReminderType::Daily, 1, 0, "Dynamic Text", true, 0},     // 5:30 PM Daily with dynamic text
        {21, 0, true, ReminderType::Daily, 1, 0, "Take your Daily Meds", false, 0},  // 9:00 PM Daily
        {21, 30, true, ReminderType::Daily, 1, 0, "Check your step count for brain health", false, 0},  // 9:30 PM Daily
        {13, 0, true, ReminderType::Weekly, 1, 6, "Dynamic Text", true, 0},  // 1:00 PM Saturday
        {13, 0, true, ReminderType::Weekly, 1, 0, "Dynamic Text", true, 0},  // 1:00 PM Sunday
      }};
      
      std::array<ReminderData, reminderCount> reminderData;

      void LoadSettingsFromFile();
      void SaveSettingsToFile() const;
      void ScheduleReminderInternal(uint8_t reminderIndex);
      void ScheduleNextReminder();
      std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> 
        CalculateNextReminderTime(uint8_t reminderIndex, 
          const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& now) const;
    };
  }
}
