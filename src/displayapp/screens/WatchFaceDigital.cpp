#include "displayapp/screens/WatchFaceDigital.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

WatchFaceDigital::WatchFaceDigital(Controllers::DateTime& dateTimeController,
                                   const Controllers::Battery& batteryController,
                                   const Controllers::Ble& bleController,
                                   const Controllers::AlarmController& alarmController,
                                   Controllers::NotificationManager& notificationManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController,
                                   Controllers::SimpleWeatherService& weatherService)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService},
    statusIcons(batteryController, bleController, alarmController),
    basePraxiomAge(53),  // Demo age - will be replaced when phone app connects
    lastSyncTime(0) {

  // Create Praxiom brand gradient background (Orange/Amber to Teal/Cyan)
  lv_obj_t* background_gradient = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(background_gradient, 240, 240);
  lv_obj_set_pos(background_gradient, 0, 0);
  lv_obj_set_style_local_radius(background_gradient, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  // Praxiom brand colors: Orange/Amber (#CC6600) to Teal (#008B8B)
  lv_obj_set_style_local_bg_color(background_gradient, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCC6600));
  lv_obj_set_style_local_bg_grad_color(background_gradient, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x008B8B));
  lv_obj_set_style_local_bg_grad_dir(background_gradient, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_VER);
  lv_obj_set_style_local_border_width(background_gradient, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

  statusIcons.Create();
  lv_obj_align(statusIcons.GetObject(), lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -8, 0);

  // Create and position Praxiom Age label (text) - BLACK for contrast
  labelPraxiomAge = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelPraxiomAge, "Praxiom Age");
  lv_obj_set_style_local_text_font(labelPraxiomAge, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(labelPraxiomAge, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_align(labelPraxiomAge, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);

  // Create Praxiom Age number - BOLD large digits - WHITE (neutral color)
  labelPraxiomAgeNumber = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelPraxiomAgeNumber, "53");  // Initial demo value
  lv_obj_set_style_local_text_font(labelPraxiomAgeNumber, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);  // BOLD font
  lv_obj_set_style_local_text_color(labelPraxiomAgeNumber, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));  // Start with white
  lv_obj_align(labelPraxiomAgeNumber, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);

  // Time label - BLACK
  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text_static(label_time, "00:00");
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 55);

  // Date label - BLACK
  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, 90);

  // Heart rate icon and label - BLACK
  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  // Step icon and label - BLACK
  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, stepIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  // Notification icon - BLACK
  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceDigital::~WatchFaceDigital() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

// Update base Praxiom Age from phone app (called via BLE)
void WatchFaceDigital::UpdateBasePraxiomAge(int age) {
  basePraxiomAge = age;
  lastSyncTime = dateTimeController.CurrentDateTime().time_since_epoch().count();
}

// Calculate real-time adjustment based on watch sensors
float WatchFaceDigital::CalculateRealtimeAdjustment() {
  float adjustment = 0.0f;
  
  // Get current sensor data
  uint8_t currentHeartRate = heartRateController.HeartRate();
  uint32_t currentSteps = motionController.NbSteps();
  
  // Heart rate deviation adjustment (-1 to +2 years based on HR)
  if (currentHeartRate > 0) {
    if (currentHeartRate < 50 || currentHeartRate > 90) {
      // Poor resting HR
      adjustment += 1.5f;
    } else if (currentHeartRate >= 50 && currentHeartRate <= 60) {
      // Excellent resting HR
      adjustment -= 0.5f;
    } else if (currentHeartRate >= 61 && currentHeartRate <= 70) {
      // Good resting HR
      adjustment -= 0.2f;
    } else if (currentHeartRate >= 71 && currentHeartRate <= 80) {
      // Fair resting HR
      adjustment += 0.3f;
    } else {
      // Suboptimal resting HR
      adjustment += 0.8f;
    }
  }
  
  // Daily activity adjustment (-1 to +1 year based on steps)
  if (currentSteps >= 10000) {
    adjustment -= 0.8f;  // Excellent activity
  } else if (currentSteps >= 8000) {
    adjustment -= 0.3f;  // Good activity
  } else if (currentSteps >= 5000) {
    adjustment += 0.2f;  // Moderate activity
  } else if (currentSteps >= 3000) {
    adjustment += 0.5f;  // Low activity
  } else {
    adjustment += 1.0f;  // Sedentary
  }
  
  // Cap adjustment to reasonable range
  if (adjustment < -2.0f) adjustment = -2.0f;
  if (adjustment > 3.0f) adjustment = 3.0f;
  
  return adjustment;
}

// Calculate final Praxiom Age
int WatchFaceDigital::GetCurrentPraxiomAge() {
  // Demo mode: basePraxiomAge = 53 until phone app connects and updates it
  // Real mode: basePraxiomAge updated by phone app via UpdateBasePraxiomAge()
  
  float adjustment = CalculateRealtimeAdjustment();
  int finalAge = basePraxiomAge + (int)adjustment;
  
  // Ensure reasonable bounds (18-120)
  if (finalAge < 18) finalAge = 18;
  if (finalAge > 120) finalAge = 120;
  
  return finalAge;
}

// Get color based on Praxiom Age difference from base age
lv_color_t WatchFaceDigital::GetPraxiomAgeColor(int currentAge, int baseAge) {
  int difference = currentAge - baseAge;
  
  if (difference < -2) {
    // More than 2 years younger - GREEN (excellent health)
    return lv_color_hex(0x00FF00);
  } else if (difference > 2) {
    // More than 2 years older - RED (poor health)
    return lv_color_hex(0xFF0000);
  } else {
    // Within ±2 years - WHITE (neutral/good health)
    return lv_color_hex(0xFFFFFF);
  }
}

void WatchFaceDigital::Refresh() {
  statusIcons.Update();

  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    // Update time
    lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      uint8_t day = dateTimeController.Day();
      lv_label_set_text_fmt(label_date,
                            "%s %d %d",
                            dateTimeController.DayOfWeekShortToString(),
                            day,
                            year);
      lv_obj_realign(label_date);
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_label_set_text_static(heartbeatValue, "");
    }
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
  }

  // Update Praxiom Age every minute with dynamic color
  static uint8_t lastMinute = 0;
  uint8_t currentMinute = dateTimeController.Minutes();
  
  if (currentMinute != lastMinute) {
    int praxiomAge = GetCurrentPraxiomAge();
    
    // Update the number
    lv_label_set_text_fmt(labelPraxiomAgeNumber, "%d", praxiomAge);
    
    // Update the color based on difference from base age
    lv_color_t ageColor = GetPraxiomAgeColor(praxiomAge, basePraxiomAge);
    lv_obj_set_style_local_text_color(labelPraxiomAgeNumber, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, ageColor);
    
    lv_obj_realign(labelPraxiomAgeNumber);
    lastMinute = currentMinute;
    
    // TODO: Send updated Praxiom Age back to phone via BLE every 10 minutes
  }
}
