#include <lvgl/lvgl.h>
#include "displayapp/screens/WatchFacePrideFlag.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFacePrideFlag*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

WatchFacePrideFlag::WatchFacePrideFlag(Controllers::DateTime& dateTimeController,
                                       const Controllers::Battery& batteryController,
                                       const Controllers::Ble& bleController,
                                       Controllers::NotificationManager& notificationManager,
                                       Controllers::Settings& settingsController,
                                       Controllers::MotionController& motionController)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController} {

  bluetoothStatus = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bluetoothStatus, "");
  lv_obj_align(bluetoothStatus, nullptr, LV_ALIGN_IN_TOP_RIGHT, -16, 0);

  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(batteryValue, true);
  lv_label_set_align(batteryValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(batteryValue, true);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_LEFT_MID, 0, -110);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblClose = lv_label_create(btnClose, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  btnNextFlag = lv_btn_create(lv_scr_act(), nullptr);
  btnNextFlag->user_data = this;
  lv_obj_set_size(btnNextFlag, 60, 60);
  lv_obj_align(btnNextFlag, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 80);
  lv_obj_set_style_local_bg_opa(btnNextFlag, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblNextBG = lv_label_create(btnNextFlag, nullptr);
  lv_label_set_text_static(lblNextBG, ">");
  lv_obj_set_event_cb(btnNextFlag, event_handler);
  lv_obj_set_hidden(btnNextFlag, true);

  btnPrevFlag = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevFlag->user_data = this;
  lv_obj_set_size(btnPrevFlag, 60, 60);
  lv_obj_align(btnPrevFlag, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 80);
  lv_obj_set_style_local_bg_opa(btnPrevFlag, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblPrevFlag = lv_label_create(btnPrevFlag, nullptr);
  lv_label_set_text_static(lblPrevFlag, "<");
  lv_obj_set_event_cb(btnPrevFlag, event_handler);
  lv_obj_set_hidden(btnPrevFlag, true);

  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelDate, true);
  lv_label_set_align(labelDate, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(labelDate, true);

  labelTime = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelTime, true);
  lv_obj_align(labelTime, lv_scr_act(), LV_ALIGN_CENTER, 0, -1);
  lv_label_set_align(labelTime, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_style_local_text_font(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_auto_realign(labelTime, true);

  labelDay = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelDay, true);
  lv_label_set_align(labelDay, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(labelDay, true);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(stepValue, true);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_CENTER, 0, 96);
  lv_label_set_align(stepValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(stepValue, true);

  UpdateScreen(settingsController.GetPrideFlag());

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFacePrideFlag::~WatchFacePrideFlag() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
  delete[] backgroundSections;
}

bool WatchFacePrideFlag::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_get_hidden(btnClose)) {
    lv_obj_set_hidden(btnPrevFlag, false);
    lv_obj_set_hidden(btnNextFlag, false);
    lv_obj_set_hidden(btnClose, false);
    savedTick = lv_tick_get();
    return true;
  }
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && (lv_obj_get_hidden(btnClose) == false)) {
    return true;
  }
  return false;
}

void WatchFacePrideFlag::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_set_hidden(btnClose, true);
  lv_obj_set_hidden(btnNextFlag, true);
  lv_obj_set_hidden(btnPrevFlag, true);
}

void WatchFacePrideFlag::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  bleState = bleController.IsConnected();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated() || themeChanged) {
    lv_label_set_text_fmt(batteryValue, "%s %d%%#", defaultTopLabelColour, batteryPercentRemaining.Get());
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValue, LV_LABEL_POS_LAST, " Charging");
    }
  }
  if (bleState.IsUpdated()) {
    if (bleState.Get()) {
      lv_label_set_text_static(bluetoothStatus, Symbols::bluetooth);
    } else {
      lv_label_set_text_static(bluetoothStatus, "");
    }
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (notificationState.Get()) {
      lv_label_set_text_static(notificationIcon, "You have\nmail!");
    } else {
      lv_label_set_text_static(notificationIcon, "");
    }
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());
  if (currentDateTime.IsUpdated() || themeChanged) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    uint8_t second = dateTimeController.Seconds();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      if (hour == 0) {
        ampmChar = "AM";
        hour = 12;
      } else if (hour == 12) {
        ampmChar = "PM";
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar = "PM";
      }
    }

    lv_label_set_text_fmt(labelTime, "%s %02d:%02d:%02d#", labelTimeColour, hour, minute, second);

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated() || ampmChar.IsUpdated() || themeChanged) {
      uint16_t year = dateTimeController.Year();
      Controllers::DateTime::Months month = dateTimeController.Month();
      uint8_t day = dateTimeController.Day();
      Controllers::DateTime::Days dayOfWeek = dateTimeController.DayOfWeek();
      lv_label_set_text_fmt(labelDate, "%s %02d-%02d-%04d#", defaultTopLabelColour, day, static_cast<uint8_t>(month), year);
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        lv_label_set_text_fmt(labelDay,
                              "%s %s %s#",
                              defaultBottomLabelColour,
                              dateTimeController.DayOfWeekToStringLow(dayOfWeek),
                              ampmChar);
      } else {
        lv_label_set_text_fmt(labelDay, "%s %s#", defaultBottomLabelColour, dateTimeController.DayOfWeekToStringLow(dayOfWeek));
      }
    }
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated() || themeChanged) {
    lv_label_set_text_fmt(stepValue, "%s %lu steps#", defaultBottomLabelColour, stepCount.Get());
  }
  if (themeChanged)
    themeChanged = false;
}

void WatchFacePrideFlag::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  auto valueFlag = settingsController.GetPrideFlag();
  bool flagChanged = false;

  if (event == LV_EVENT_CLICKED) {
    if (object == btnClose) {
      CloseMenu();
    }
    if (object == btnNextFlag) {
      valueFlag = GetNext(valueFlag);
      flagChanged = true;
    }
    if (object == btnPrevFlag) {
      valueFlag = GetPrevious(valueFlag);
      flagChanged = true;
    }
    settingsController.SetPrideFlag(valueFlag);
    if (flagChanged) {
      UpdateScreen(valueFlag);
    }
  }
}

bool WatchFacePrideFlag::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnClose)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFacePrideFlag::UpdateScreen(Pinetime::Controllers::Settings::PrideFlag prideFlag) {
  themeChanged = true;
  auto prideFlagAsInt = static_cast<uint8_t>(prideFlag);
  if (initialized) {
    for (int i = 0; i < numBackgrounds; i++) {
      lv_obj_del(backgroundSections[i]);
    }
    delete[] backgroundSections;
  }
  initialized = true;
  switch (prideFlagAsInt) {
    case 0:
      numBackgrounds = 7;
      backgroundSections = new lv_obj_t*[numBackgrounds];
      for (int i = 0; i < numBackgrounds; i++) {
        backgroundSections[i] = lv_obj_create(lv_scr_act(), nullptr);
        lv_obj_set_size(backgroundSections[i], LV_HOR_RES, (LV_VER_RES / numBackgrounds) + 1);
        lv_obj_set_pos(backgroundSections[i], 0, i * LV_VER_RES / numBackgrounds);
        lv_obj_set_style_local_radius(backgroundSections[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_obj_move_background(backgroundSections[i]);
      }
      lv_obj_set_style_local_bg_color(backgroundSections[0], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, darkGreen);
      lv_obj_set_style_local_bg_color(backgroundSections[1], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, cyan);
      lv_obj_set_style_local_bg_color(backgroundSections[2], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightGreen);
      lv_obj_set_style_local_bg_color(backgroundSections[3], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_bg_color(backgroundSections[4], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightBlue);
      lv_obj_set_style_local_bg_color(backgroundSections[5], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, indigo);
      lv_obj_set_style_local_bg_color(backgroundSections[6], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, steelBlue);
      lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -102);
      lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_CENTER, 0, -51);
      lv_obj_align(labelDay, lv_scr_act(), LV_ALIGN_CENTER, 0, 51);
      lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_CENTER, 0, 102);
      strncpy(labelTimeColour, "#000000", 7);
      strncpy(defaultTopLabelColour, "#000000", 7);
      strncpy(defaultBottomLabelColour, "#ffffff", 7);
      break;
    case 1:
      numBackgrounds = 5;
      backgroundSections = new lv_obj_t*[numBackgrounds];
      for (int i = 0; i < numBackgrounds; i++) {
        backgroundSections[i] = lv_obj_create(lv_scr_act(), nullptr);
        lv_obj_set_size(backgroundSections[i], LV_HOR_RES, LV_VER_RES / numBackgrounds);
        lv_obj_set_pos(backgroundSections[i], 0, i * LV_VER_RES / numBackgrounds);
        lv_obj_set_style_local_radius(backgroundSections[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_obj_move_background(backgroundSections[i]);
      }
      lv_obj_set_style_local_bg_color(backgroundSections[0], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightBlue);
      lv_obj_set_style_local_bg_color(backgroundSections[1], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightPink);
      lv_obj_set_style_local_bg_color(backgroundSections[2], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_bg_color(backgroundSections[3], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightPink);
      lv_obj_set_style_local_bg_color(backgroundSections[4], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightBlue);
      lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -96);
      lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_CENTER, 0, -48);
      lv_obj_align(labelDay, lv_scr_act(), LV_ALIGN_CENTER, 0, 48);
      lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_CENTER, 0, 96);
      strncpy(labelTimeColour, "#000000", 7);
      strncpy(defaultTopLabelColour, "#ffffff", 7);
      strncpy(defaultBottomLabelColour, "#ffffff", 7);
      break;
    case 2:
      numBackgrounds = 5;
      backgroundSections = new lv_obj_t*[numBackgrounds];
      for (int i = 0; i < numBackgrounds; i++) {
        backgroundSections[i] = lv_obj_create(lv_scr_act(), nullptr);
        lv_obj_set_size(backgroundSections[i], LV_HOR_RES, LV_VER_RES / numBackgrounds);
        lv_obj_set_pos(backgroundSections[i], 0, i * LV_VER_RES / numBackgrounds);
        lv_obj_set_style_local_radius(backgroundSections[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_obj_move_background(backgroundSections[i]);
      }
      lv_obj_set_style_local_bg_color(backgroundSections[0], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hotPink);
      lv_obj_set_style_local_bg_color(backgroundSections[1], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, hotPink);
      lv_obj_set_style_local_bg_color(backgroundSections[2], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, grayPurple);
      lv_obj_set_style_local_bg_color(backgroundSections[3], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, darkBlue);
      lv_obj_set_style_local_bg_color(backgroundSections[4], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, darkBlue);
      lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -96);
      lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_CENTER, 0, -48);
      lv_obj_align(labelDay, lv_scr_act(), LV_ALIGN_CENTER, 0, 48);
      lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_CENTER, 0, 96);
      strncpy(labelTimeColour, "#ffffff", 7);
      strncpy(defaultTopLabelColour, "#000000", 7);
      strncpy(defaultBottomLabelColour, "#000000", 7);
      break;
    case 3:
      numBackgrounds = 7;
      backgroundSections = new lv_obj_t*[numBackgrounds];
      for (int i = 0; i < numBackgrounds; i++) {
        backgroundSections[i] = lv_obj_create(lv_scr_act(), nullptr);
        lv_obj_set_size(backgroundSections[i], LV_HOR_RES, (LV_VER_RES / numBackgrounds) + 1);
        lv_obj_set_pos(backgroundSections[i], 0, i * LV_VER_RES / numBackgrounds);
        lv_obj_set_style_local_radius(backgroundSections[i], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_obj_move_background(backgroundSections[i]);
      }
      lv_obj_set_style_local_bg_color(backgroundSections[0], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
      lv_obj_set_style_local_bg_color(backgroundSections[1], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, orange);
      lv_obj_set_style_local_bg_color(backgroundSections[2], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightOrange);
      lv_obj_set_style_local_bg_color(backgroundSections[3], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_bg_color(backgroundSections[4], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lightPurple);
      lv_obj_set_style_local_bg_color(backgroundSections[5], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, darkPurple);
      lv_obj_set_style_local_bg_color(backgroundSections[6], LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, magenta);
      lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -102);
      lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_CENTER, 0, -51);
      lv_obj_align(labelDay, lv_scr_act(), LV_ALIGN_CENTER, 0, 51);
      lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_CENTER, 0, 102);
      strncpy(labelTimeColour, "#000000", 7);
      strncpy(defaultTopLabelColour, "#ffffff", 7);
      strncpy(defaultBottomLabelColour, "#ffffff", 7);
      break;
  }
}

Pinetime::Controllers::Settings::PrideFlag WatchFacePrideFlag::GetNext(Pinetime::Controllers::Settings::PrideFlag prideFlag) {
  auto prideFlagAsInt = static_cast<uint8_t>(prideFlag);
  Pinetime::Controllers::Settings::PrideFlag nextFlag;
  if (prideFlagAsInt < 3) {
    nextFlag = static_cast<Controllers::Settings::PrideFlag>(prideFlagAsInt + 1);
  } else {
    nextFlag = static_cast<Controllers::Settings::PrideFlag>(0);
  }
  return nextFlag;
}

Pinetime::Controllers::Settings::PrideFlag WatchFacePrideFlag::GetPrevious(Pinetime::Controllers::Settings::PrideFlag prideFlag) {
  auto prideFlagAsInt = static_cast<uint8_t>(prideFlag);
  Pinetime::Controllers::Settings::PrideFlag prevFlag;
  if (prideFlagAsInt > 0) {
    prevFlag = static_cast<Controllers::Settings::PrideFlag>(prideFlagAsInt - 1);
  } else {
    prevFlag = static_cast<Controllers::Settings::PrideFlag>(3);
  }
  return prevFlag;
}
