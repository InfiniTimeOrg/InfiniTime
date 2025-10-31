#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include <array>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ScheduledReminders : public Screen {
      public:
        ScheduledReminders(Pinetime::Applications::AppControllers& controllers);
        ~ScheduledReminders() override;
        
        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

      private:
        void CreateContainer();
        void CreateMainToggle();
        void CreateReminderList();
        void CreateReminderItem(uint8_t index);
        void UpdateMainToggleLabel();
        void UpdateMainToggleColor();
        void UpdateReminderStatus();
        void ToggleAllReminders();
        
        // Alerting reminder functionality
        void CreateAlertingReminderDisplay();
        // void UpdateAlertingReminderDisplay();
        void DismissAlertingReminder();
        uint8_t GetAlertingReminderIndex() const;
        bool HasAlertingReminder() const;
        
        Pinetime::Applications::AppControllers& controllers;
        lv_obj_t* container;
        lv_obj_t* subTitle;
        lv_obj_t* mainToggleButton;
        lv_obj_t* mainToggleLabel;
        lv_obj_t* reminderListContainer;
        std::array<lv_obj_t*, 4> reminderItems;
        std::array<lv_obj_t*, 4> timeLabels;
        std::array<lv_obj_t*, 4> nameLabels;
        std::array<lv_obj_t*, 4> statusIndicators;
        
        // Alerting reminder UI elements
        lv_obj_t* alertingContainer;
        lv_obj_t* alertingTitle;
        lv_obj_t* alertingMessage;
        lv_obj_t* alertingTime;
        // lv_obj_t* dismissButton;
        // lv_obj_t* dismissLabel;
        
        static constexpr uint8_t maxReminders = 4;
      };
    }

    template <>
    struct AppTraits<Apps::ScheduledReminders> {
      static constexpr Apps app = Apps::ScheduledReminders;
      static constexpr const char* icon = Screens::Symbols::clock;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::ScheduledReminders(controllers);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}