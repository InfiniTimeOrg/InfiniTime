#include "displayapp/screens/WatchFaceLCARS.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/BleIcon.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "components/heartrate/HeartRateController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void set_label_text_from_uint(lv_obj_t* label, uint8_t number) {
    std::string number_as_string = std::to_string(number);
    char* number_as_char = new char[number_as_string.length() +1];
    std::strcpy(number_as_char, number_as_string.c_str());
    lv_label_set_text_static(label, number_as_char);
  }
  void set_label_color(lv_obj_t* label, lv_color_t color) {
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
  }
  lv_obj_t* label_make(lv_obj_t* container, uint8_t position_x, uint8_t position_y, lv_color_t color, uint8_t align, const char* text) {
    lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
    lv_label_set_text_static(label, text);
    lv_obj_align(label, container, align, position_x, position_y);
    return label;
  }
  lv_obj_t* label_make_with_font(lv_obj_t* container, uint8_t position_x, uint8_t position_y, lv_color_t color, lv_font_t* font, uint8_t align, const char* text) {
    lv_obj_t* label = label_make(container, position_x, position_y, color, align, text);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_realign(label);
    return label;
  }
  lv_obj_t* label_container_make(lv_obj_t* parent, uint8_t position_x, uint8_t position_y, uint8_t size_x, uint8_t size_y, uint8_t align) {
    lv_obj_t* container = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_bg_opa(container, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_size(container, size_x, size_y);
    lv_obj_align(container, parent, align, position_x, position_y);
    return container;
  }
}

WatchFaceLCARS::WatchFaceLCARS(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::MotionController& motionController,
                                     Controllers::HeartRateController& heartRateController,
                                     Controllers::FS& filesystem)
  : currentDateTime {{}},
    currentNanoSeconds {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController},
    heartRateController {heartRateController} {

  // Fonts
  lfs_file f = {};
  
  if (filesystem.FileOpen(&f, "/fonts/antonio_78.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_antonio_78 = lv_font_load("F:/fonts/antonio_78.bin");
  }
  if (filesystem.FileOpen(&f, "/fonts/antonio_33.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_antonio_33 = lv_font_load("F:/fonts/antonio_33.bin");
  }
  if (filesystem.FileOpen(&f, "/fonts/antonio_21.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_antonio_21 = lv_font_load("F:/fonts/antonio_21.bin");
  }
  if (filesystem.FileOpen(&f, "/fonts/antonio_13.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_antonio_13 = lv_font_load("F:/fonts/antonio_13.bin");
  }
  
  // Background
  background = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(background, "F:/images/LCARS.bin");
  lv_obj_set_pos(background, 0, 0);
  
  // System
  system_container = label_container_make(lv_scr_act(), 0, 5, 170, 20, LV_ALIGN_IN_TOP_RIGHT);
  lv_obj_align(system_container, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 5); // TODO: Find out why this align is necessary
  labelBattery = label_make_with_font(system_container, 0, 0, grayColor, font_antonio_21, LV_ALIGN_IN_TOP_RIGHT, "0 %");
  bleIcon = label_make(labelBattery, -5, 0, orangeColor, LV_ALIGN_OUT_LEFT_MID, Symbols::bluetooth);
  lv_obj_align(bleIcon, labelBattery, LV_ALIGN_OUT_LEFT_MID, -5, 0); // TODO: Find out why this align is necessary

  // Date
  dateContainer = label_container_make(lv_scr_act(), 65, 50, 175, 20, LV_ALIGN_IN_TOP_LEFT);
  labelDate = label_make_with_font(dateContainer, 0, 0, orangeColor, font_antonio_21, LV_ALIGN_IN_LEFT_MID, "12024 MON 01 MAR");
  labelTimeAmPm = label_make_with_font(dateContainer, 0, 0, orangeColor, font_antonio_21, LV_ALIGN_IN_RIGHT_MID, "");

  // Seconds Labels  
  label_seconds_container = label_container_make(lv_scr_act(), 65, 150, 90, 15, LV_ALIGN_IN_TOP_LEFT);
  label_tens_container = label_container_make(label_seconds_container, 0, 0, 90, 15, LV_ALIGN_IN_TOP_LEFT);
  for (int i = 0; i < 6; ++i) {
    label_tens[i] = label_make_with_font(label_tens_container, 10 * i, 0, grayColor, font_antonio_13, LV_ALIGN_IN_LEFT_MID, "");
    set_label_text_from_uint(label_tens[i], i);
  }
  label_ones_container = label_container_make(label_seconds_container, 0, 15, 90, 15, LV_ALIGN_IN_TOP_LEFT);
  for (int i = 0; i < 10; ++i) {
    label_ones[i] = label_make_with_font(label_ones_container, 10 * i, 0, grayColor, font_antonio_13, LV_ALIGN_IN_LEFT_MID, "");
    set_label_text_from_uint(label_ones[i], i);
  }

  // Time
  timeContainer = label_container_make(lv_scr_act(), 65, 76, 170, 60, LV_ALIGN_IN_TOP_LEFT);
  labelTime = label_make_with_font(timeContainer, 0, 0, orangeColor, font_antonio_78, LV_ALIGN_IN_TOP_LEFT, "00:00");

  // WK
  label_week = label_make_with_font(lv_scr_act(), -5, 150, orangeColor, font_antonio_33, LV_ALIGN_IN_TOP_RIGHT, "WK00");
  lv_obj_align(label_week, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 150); // TODO: Find out why this align is necessary

  // Sensors
  sensors_container = label_container_make(lv_scr_act(), 0, 0, 130, 50, LV_ALIGN_IN_BOTTOM_RIGHT);
  lv_obj_align(sensors_container, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // TODO: Find out why this align is necessary
  stepValue = label_make_with_font(sensors_container, -5, 0, orangeColor, font_antonio_21, LV_ALIGN_IN_BOTTOM_RIGHT, "0");
  lv_obj_align(stepValue, sensors_container, LV_ALIGN_IN_BOTTOM_RIGHT, -5, 0); // TODO: Find out why this align is necessary
  stepIcon = label_make(stepValue, -5, 0, orangeColor, LV_ALIGN_OUT_LEFT_MID, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0); // TODO: Find out why this align is necessary
  heartbeatValue = label_make_with_font(sensors_container, -5, -25, orangeColor, font_antonio_21, LV_ALIGN_IN_BOTTOM_RIGHT, "0");
  lv_obj_align(heartbeatValue, sensors_container, LV_ALIGN_IN_BOTTOM_RIGHT, -5, -25); // TODO: Find out why this align is necessary
  heartbeatIcon = label_make(stepValue, -25, 0, orangeColor, LV_ALIGN_OUT_LEFT_MID, "");
  lv_obj_align(heartbeatIcon, heartbeatValue, LV_ALIGN_IN_BOTTOM_LEFT, -25, 0); // TODO: Find out why this align is necessary

  // Tasks
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceLCARS::~WatchFaceLCARS() {
  // Tasks
  lv_task_del(taskRefresh);
  
  // Fonts
  if (font_antonio_78 != nullptr) {
    lv_font_free(font_antonio_78);
  }
  if (font_antonio_33 != nullptr) {
    lv_font_free(font_antonio_33);
  }
  if (font_antonio_21 != nullptr) {
    lv_font_free(font_antonio_21);
  }
  if (font_antonio_13 != nullptr) {
    lv_font_free(font_antonio_13);
  }
  
  // Objects
  lv_obj_clean(lv_scr_act());
}

void WatchFaceLCARS::Refresh() {
  currentNanoSeconds = std::chrono::time_point_cast<std::chrono::nanoseconds>(dateTimeController.CurrentDateTime());
  if (currentNanoSeconds.IsUpdated()) {
    UpdateSeconds();
    UpdateStepCount();
    UpdateBatteryPercent();
    UpdateBLE();
    UpdateHeartRate();
    currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(currentNanoSeconds.Get());
    if (currentDateTime.IsUpdated()) {
      UpdateTime();
      currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
      if (currentDate.IsUpdated()) {
        UpdateStardate();
        UpdateWK();
      }
    }
  }
}

void WatchFaceLCARS::UpdateHeartRate() {
  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeat.Get() > 120) {
      set_label_color(heartbeatValue, redColor);
      set_label_color(heartbeatIcon, redColor);
    } else {
      set_label_color(heartbeatValue, orangeColor);
      set_label_color(heartbeatIcon, orangeColor);
    }
    if (heartbeatRunning.Get()) {
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
      lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
    } else {
      lv_label_set_text_static(heartbeatValue, "");
      lv_label_set_text_static(heartbeatIcon, "");
    }
    lv_obj_realign(heartbeatValue);
    lv_obj_realign(heartbeatIcon);
  }
}

void WatchFaceLCARS::UpdateTime() {
  uint8_t hour = dateTimeController.Hours();
  uint8_t minute = dateTimeController.Minutes();

  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    char ampmChar[3] = "AM";
    if (hour == 0) {
      hour = 12;
    } else if (hour == 12) {
      ampmChar[0] = 'P';
    } else if (hour > 12) {
      hour = hour - 12;
      ampmChar[0] = 'P';
    }
    lv_label_set_text(labelTimeAmPm, ampmChar);
    weekNumberFormat = "%V";
  } else {
    weekNumberFormat = "%U";
  }
  lv_label_set_text_fmt(labelTime, "%2d:%02d", hour, minute);

  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    lv_obj_realign(labelTimeAmPm);
    lv_obj_realign(labelTime);
  }
}

void WatchFaceLCARS::UpdateSeconds() {
  uint8_t second = dateTimeController.Seconds();
  if (last_second != second) {
    last_second = second;
    ResetSecondsDigits();
    set_label_color(label_tens[second / 10], orangeColor);
    set_label_color(label_ones[second % 10], orangeColor);
  }
}

void WatchFaceLCARS::UpdateWK() {
  time_t ttTime = std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(currentDate.Get()));
  tm* tmTime = std::localtime(&ttTime);
  char buffer[8];
  strftime(buffer, 8, weekNumberFormat, tmTime);
  uint8_t weekNumber = atoi(buffer);
  lv_label_set_text_fmt(label_week, "WK%02d", weekNumber);
  lv_obj_realign(label_week);
}

void WatchFaceLCARS::UpdateStardate() {
  lv_label_set_text_fmt(
    labelDate,
    "1%d %s %d %s",
    dateTimeController.Year(),
    dateTimeController.DayOfWeekShortToString(),
    dateTimeController.Day(),
    dateTimeController.MonthShortToString()
  );
  lv_obj_realign(labelDate);
}

void WatchFaceLCARS::UpdateBatteryPercent() {
  batteryPercentRemaining = batteryController.PercentRemaining();
  isCharging = batteryController.IsCharging();
  if (batteryController.IsCharging() ) { // Charging battery animation
    chargingBatteryPercent += 1;
    if (chargingBatteryPercent > 100) {
      chargingBatteryPercent = batteryPercentRemaining.Get();
    }
    if (chargingBatteryPercent <= 100 && ((chargingBatteryPercent % 47) == 0)) {
      SetBatteryLevel(batteryPercentRemaining.Get(), orangeColor);
    }
    if ((chargingBatteryPercent % 97) == 0) {
      SetBatteryLevel(batteryPercentRemaining.Get(), grayColor);
    }
  } else if (isCharging.IsUpdated() || batteryPercentRemaining.IsUpdated()) {
    chargingBatteryPercent = batteryPercentRemaining.Get();
    if (chargingBatteryPercent <= 20) {
      SetBatteryLevel(chargingBatteryPercent, redColor);
    } else if (chargingBatteryPercent > 99) {
      SetBatteryLevel(chargingBatteryPercent, orangeColor);
    } else {
      SetBatteryLevel(chargingBatteryPercent, grayColor);
    }
  }
}

void WatchFaceLCARS::UpdateBLE() {
  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
    lv_obj_realign(bleIcon);
  }
}

void WatchFaceLCARS::UpdateStepCount() {
  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepIcon);
  }
}

void WatchFaceLCARS::ResetSecondsDigits() {
  for (int i = 0; i < 6; ++i) {
    set_label_color(label_tens[i], grayColor);
  }
  for (int i = 0; i < 10; ++i) {
    set_label_color(label_ones[i], grayColor);
  }
}

void WatchFaceLCARS::SetBatteryLevel(uint8_t batteryPercent, const lv_color_t& color) {
  lv_label_set_text_fmt(labelBattery, "%lu%%", batteryPercent);
  set_label_color(labelBattery, color);
  lv_obj_realign(labelBattery);
  lv_obj_realign(bleIcon);
}

bool WatchFaceLCARS::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/antonio_78.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/antonio_33.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/antonio_21.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/antonio_13.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);
  return true;
}
