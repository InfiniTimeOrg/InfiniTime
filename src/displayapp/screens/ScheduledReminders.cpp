#include "displayapp/screens/ScheduledReminders.h"
#include "displayapp/DisplayApp.h"
#include "components/scheduledreminders/ScheduledRemindersController.h"
#include <sstream>
#include <iomanip>

using namespace Pinetime::Applications::Screens;

ScheduledReminders::ScheduledReminders(Pinetime::Applications::AppControllers& controllers) : controllers {controllers} {
  // Initialize alerting UI elements to nullptr
  alertingContainer = nullptr;
  alertingTitle = nullptr;
  alertingMessage = nullptr;
  alertingTime = nullptr;
  dismissButton = nullptr;
  dismissLabel = nullptr;
  
  CreateContainer();
  
  // Check if there's an alerting reminder first
  if (HasAlertingReminder()) {
    CreateAlertingReminderDisplay();
  } else {
    // Create normal interface
    lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_text_static(title, "Hearing Reminders");
    lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

    subTitle = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_width(subTitle, LV_HOR_RES); // Set width to full screen width
    lv_label_set_align(subTitle, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(subTitle, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 35);
    
    // Create main toggle button
    CreateMainToggle();
  }
}

ScheduledReminders::~ScheduledReminders() {
  lv_obj_clean(lv_scr_act());
}

void ScheduledReminders::CreateContainer() {
  container = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_border_width(container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(container, 200, 180);
  lv_obj_align(container, lv_scr_act(), LV_ALIGN_CENTER, 0, 10);
}

void ScheduledReminders::CreateMainToggle() {
  // Create main toggle button
  mainToggleButton = lv_btn_create(container, nullptr);
  lv_obj_set_size(mainToggleButton, 160, 40);
  lv_obj_align(mainToggleButton, container, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  
  // Create toggle label
  mainToggleLabel = lv_label_create(mainToggleButton, nullptr);
  UpdateMainToggleLabel();
  lv_obj_align(mainToggleLabel, mainToggleButton, LV_ALIGN_CENTER, 0, 0);
  
  // Set initial button color
  UpdateMainToggleColor();
}

void ScheduledReminders::CreateReminderList() {
  // Create reminder list container
  reminderListContainer = lv_obj_create(container, nullptr);
  lv_obj_set_style_local_bg_color(reminderListContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_border_width(reminderListContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_border_color(reminderListContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_size(reminderListContainer, 180, 100);
  lv_obj_align(reminderListContainer, container, LV_ALIGN_IN_TOP_MID, 0, 70);
  
  // Create reminder items (display only, no individual toggles)
  // for (uint8_t i = 0; i < maxReminders; i++) {
  //   CreateReminderItem(i);
  // }
}

// void ScheduledReminders::CreateReminderItem(uint8_t index) {
//   if (index >= maxReminders) return;
  
//   // Create reminder item container
//   reminderItems[index] = lv_obj_create(reminderListContainer, nullptr);
//   lv_obj_set_style_local_bg_color(reminderItems[index], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
//   lv_obj_set_style_local_border_width(reminderItems[index], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
//   lv_obj_set_size(reminderItems[index], 160, 20);
//   lv_obj_align(reminderItems[index], reminderListContainer, LV_ALIGN_IN_TOP_MID, 0, 5 + index * 22);
  
//   // Create name label
//   nameLabels[index] = lv_label_create(reminderItems[index], nullptr);
//   lv_label_set_text_static(nameLabels[index], controllers.scheduledRemindersController.GetReminderName(index));
//   lv_obj_align(nameLabels[index], reminderItems[index], LV_ALIGN_IN_LEFT_MID, 5, 0);
  
//   // Create time label
//   timeLabels[index] = lv_label_create(reminderItems[index], nullptr);
//   std::stringstream timeStr;
//   timeStr << std::setfill('0') << std::setw(2) << static_cast<int>(controllers.scheduledRemindersController.GetReminderHours(index))
//           << ":" << std::setfill('0') << std::setw(2) << static_cast<int>(controllers.scheduledRemindersController.GetReminderMinutes(index));
//   lv_label_set_text(timeLabels[index], timeStr.str().c_str());
//   lv_obj_align(timeLabels[index], reminderItems[index], LV_ALIGN_IN_RIGHT_MID, -5, 0);
  
//   // Create status indicator (small circle)
//   statusIndicators[index] = lv_obj_create(reminderItems[index], nullptr);
//   lv_obj_set_size(statusIndicators[index], 8, 8);
//   lv_obj_align(statusIndicators[index], reminderItems[index], LV_ALIGN_IN_LEFT_MID, 80, 0);
//   lv_obj_set_style_local_radius(statusIndicators[index], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 4);
  
//   // Set status indicator color based on enabled state
//   if (controllers.scheduledRemindersController.IsReminderEnabled(index)) {
//     lv_obj_set_style_local_bg_color(statusIndicators[index], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
//   } else {
//     lv_obj_set_style_local_bg_color(statusIndicators[index], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
//   }
// }

void ScheduledReminders::UpdateMainToggleLabel() {
  if (controllers.scheduledRemindersController.AreAllRemindersEnabled()) {
    lv_label_set_text_static(subTitle, "are Enabled");
    lv_label_set_text_static(mainToggleLabel, "Disable");
  } else {
    lv_label_set_text_static(subTitle, "are Disabled");
    lv_label_set_text_static(mainToggleLabel, "Enable");
  }
  lv_label_set_align(subTitle, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(subTitle, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 35);
}

void ScheduledReminders::UpdateMainToggleColor() {
  if (controllers.scheduledRemindersController.AreAllRemindersEnabled()) {
    lv_obj_set_style_local_bg_color(mainToggleButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  } else {
    lv_obj_set_style_local_bg_color(mainToggleButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  }
}

void ScheduledReminders::UpdateReminderStatus() {
  // Update all status indicators
  // for (uint8_t i = 0; i < maxReminders; i++) {
  //   if (controllers.scheduledRemindersController.IsReminderEnabled(i)) {
  //     lv_obj_set_style_local_bg_color(statusIndicators[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  //   } else {
  //     lv_obj_set_style_local_bg_color(statusIndicators[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  //   }
  // }
  
  // Update main toggle
  UpdateMainToggleLabel();
  UpdateMainToggleColor();
}

void ScheduledReminders::ToggleAllReminders() {
  controllers.scheduledRemindersController.ToggleAllReminders();
  UpdateReminderStatus();
}

bool ScheduledReminders::OnTouchEvent(TouchEvents event) {
  switch (event) {
    case TouchEvents::Tap:
      if (HasAlertingReminder()) {
        // If there's an alerting reminder, dismiss it and show normal interface
        DismissAlertingReminder();
        return true; // Handled the touch event
      } else {
        // Normal toggle behavior
        ToggleAllReminders();
        return true;
      }
    default:
      break;
  }
  return false;
}

bool ScheduledReminders::OnButtonPushed() {
  if (HasAlertingReminder()) {
    // If there's an alerting reminder, dismiss it and show normal interface
    DismissAlertingReminder();
    return true; // Handled the button press
  }
  return false; // Let the system handle as back navigation
}

// Alerting reminder functionality
bool ScheduledReminders::HasAlertingReminder() const {
  for (uint8_t i = 0; i < controllers.scheduledRemindersController.GetReminderCount(); i++) {
    if (controllers.scheduledRemindersController.IsReminderAlerting(i)) {
      return true;
    }
  }
  return false;
}

uint8_t ScheduledReminders::GetAlertingReminderIndex() const {
  for (uint8_t i = 0; i < controllers.scheduledRemindersController.GetReminderCount(); i++) {
    if (controllers.scheduledRemindersController.IsReminderAlerting(i)) {
      return i;
    }
  }
  return 0; // Should not reach here if HasAlertingReminder() is called first
}

void ScheduledReminders::CreateAlertingReminderDisplay() {
  uint8_t alertingIndex = GetAlertingReminderIndex();
  
  // Create alerting container - make it larger to accommodate longer text
  alertingContainer = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(alertingContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_border_width(alertingContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_border_color(alertingContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_set_size(alertingContainer, 220, 200); // Increased size for longer text
  lv_obj_align(alertingContainer, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  
  // Create reminder message with text wrapping support
  alertingMessage = lv_label_create(alertingContainer, nullptr);
  lv_label_set_text(alertingMessage, controllers.scheduledRemindersController.GetReminderName(alertingIndex));
  lv_label_set_long_mode(alertingMessage, LV_LABEL_LONG_BREAK); // Enable text wrapping
  lv_obj_set_width(alertingMessage, 200); // Set width to allow wrapping within container
  lv_label_set_align(alertingMessage, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(alertingMessage, alertingContainer, LV_ALIGN_IN_TOP_MID, 0, 10);
  
  // Create dismiss button
  dismissButton = lv_btn_create(alertingContainer, nullptr);
  lv_obj_set_size(dismissButton, 120, 40);
  lv_obj_align(dismissButton, alertingContainer, LV_ALIGN_IN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_local_bg_color(dismissButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  
  // Create dismiss label
  dismissLabel = lv_label_create(dismissButton, nullptr);
  lv_label_set_text_static(dismissLabel, "Dismiss");
  lv_label_set_align(dismissLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(dismissLabel, dismissButton, LV_ALIGN_CENTER, 0, 0);
}

// void ScheduledReminders::UpdateAlertingReminderDisplay() {
//   if (!HasAlertingReminder()) {
//     return; // No alerting reminder to update
//   }
  
//   uint8_t alertingIndex = GetAlertingReminderIndex();
  
//   // Update reminder name
//   lv_label_set_text(alertingMessage, controllers.scheduledRemindersController.GetReminderName(alertingIndex));
  
//   // // Update time display
//   // std::stringstream timeStr;
//   // timeStr << std::setfill('0') << std::setw(2) << static_cast<int>(controllers.scheduledRemindersController.GetReminderHours(alertingIndex))
//   //         << ":" << std::setfill('0') << std::setw(2) << static_cast<int>(controllers.scheduledRemindersController.GetReminderMinutes(alertingIndex));
//   // lv_label_set_text(alertingTime, timeStr.str().c_str());
// }

void ScheduledReminders::DismissAlertingReminder() {
  if (!HasAlertingReminder()) {
    return; // No alerting reminder to dismiss
  }
  
  // Stop the alerting reminder
  controllers.scheduledRemindersController.StopAlerting();
  
  // Clean up alerting UI
  if (alertingContainer) {
    lv_obj_del(alertingContainer);
    alertingContainer = nullptr;
    alertingTitle = nullptr;
    alertingMessage = nullptr;
    alertingTime = nullptr;
    dismissButton = nullptr;
    dismissLabel = nullptr;
  }
  
  // Create a minimal normal interface to prevent blank screen
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Hearing Reminders");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  subTitle = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_width(subTitle, LV_HOR_RES);
  
  // Update the subtitle to show current state
  if (controllers.scheduledRemindersController.AreAllRemindersEnabled()) {
    lv_label_set_text_static(subTitle, "are Enabled");
  } else {
    lv_label_set_text_static(subTitle, "are Disabled");
  }
  lv_label_set_align(subTitle, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(subTitle, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 35);

  CreateMainToggle();
}