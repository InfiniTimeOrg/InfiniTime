#include "displayapp/screens/WatchFaceDigital.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFaceDigital*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

WatchFaceDigital::WatchFaceDigital(DisplayApp* app,
                                   Controllers::DateTime& dateTimeController,
                                   Controllers::Battery& batteryController,
                                   Controllers::Ble& bleController,
                                   Controllers::NotificationManager& notificatioManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController} {

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(batteryValue, "");
  lv_obj_align(batteryValue, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryPlug, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));
  lv_label_set_text_static(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0082FC));
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);

  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -30, -55);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  container1 = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(container1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_pos(container1, 10, 10);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 20);

  btnSet = lv_btn_create(container1, nullptr);
  btnSet->user_data = this;
  lv_obj_set_height(btnSet, 150);
  lv_obj_set_width(btnSet, 150);
  lv_obj_align(btnSet, container1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_radius(btnSet, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 30);
  lv_obj_set_style_local_bg_opa(btnSet, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_event_cb(btnSet, event_handler);
  lbl_btnSet = lv_label_create(btnSet, nullptr);
  lv_obj_set_style_local_text_font(lbl_btnSet, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(lbl_btnSet, Symbols::settings);
  lv_obj_set_hidden(btnSet, true);

  settingsTitle = lv_label_create(container1, nullptr);
  lv_label_set_text_static(settingsTitle, "Display Settings");
  lv_label_set_align(settingsTitle, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(settingsTitle, container1, LV_ALIGN_IN_TOP_MID, 0, 15);
  lv_obj_set_hidden(settingsTitle, true);

  optionsTotal = 0;
  cbOption[optionsTotal] = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text_static(cbOption[optionsTotal], " Show Percent");
  cbOption[optionsTotal]->user_data = this;
  lv_obj_set_event_cb(cbOption[optionsTotal], event_handler);
  if (settingsController.GetDWSBatteryPercentageStatus() == Controllers::Settings::BatteryPercentage::ON) {
    lv_checkbox_set_checked(cbOption[optionsTotal], true);
  }
  lv_obj_align(cbOption[optionsTotal], settingsTitle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
  lv_obj_set_hidden(cbOption[optionsTotal], true);
  optionsTotal++;

  cbOption[optionsTotal] = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text_static(cbOption[optionsTotal], " Color Icon");
  cbOption[optionsTotal]->user_data = this;
  lv_obj_set_event_cb(cbOption[optionsTotal], event_handler);
  if (settingsController.GetDWSBatteryColorStatus() == Controllers::Settings::BatteryColor::ON) {
    lv_checkbox_set_checked(cbOption[optionsTotal], true);
  }
  lv_obj_align(cbOption[optionsTotal], cbOption[optionsTotal-1], LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
  lv_obj_set_hidden(cbOption[optionsTotal], true);
  optionsTotal++;

  btnClose = lv_btn_create(container1, nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, container1, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
  lv_obj_set_style_local_value_str(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceDigital::~WatchFaceDigital() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceDigital::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_label_set_text_static(batteryPlug, BatteryIcon::GetPlugIcon(powerPresent.Get()));
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();

    if (settingsController.GetDWSBatteryColorStatus() == Controllers::Settings::BatteryColor::ON) {
      lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BatteryIcon::GetBatteryColor(batteryPercent));
    } else {
      lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BatteryIcon::GetDefaultBatteryColor(batteryPercent));
    }
    
    if (settingsController.GetDWSBatteryPercentageStatus() == Controllers::Settings::BatteryPercentage::ON) {
      lv_label_set_text_fmt(batteryValue, "%lu%%", batteryPercent);
    } else {
      lv_label_set_text_static(batteryValue, "");
    }

    lv_label_set_text_static(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text(bleIcon, BleIcon::GetIcon(bleRadioEnabled.Get(), bleState.Get()));
  }
  lv_obj_realign(batteryIcon);
  lv_obj_realign(batteryValue);
  lv_obj_realign(batteryPlug);
  lv_obj_realign(bleIcon);

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = dateTimeController.CurrentDateTime();

  if (currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime - dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto year = static_cast<int>(yearMonthDay.year());
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>(static_cast<unsigned>(yearMonthDay.month()));
    auto day = static_cast<unsigned>(yearMonthDay.day());
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    uint8_t hour = time.hours().count();
    uint8_t minute = time.minutes().count();

    if (displayedHour != hour || displayedMinute != minute) {
      displayedHour = hour;
      displayedMinute = minute;

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
        lv_label_set_text(label_time_ampm, ampmChar);
        lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
        lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);
      } else {
        lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
        lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
      }
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(
          label_date, "%s %d %s %d", dateTimeController.DayOfWeekShortToString(), day, dateTimeController.MonthShortToString(), year);
      } else {
        lv_label_set_text_fmt(
          label_date, "%s %s %d %d", dateTimeController.DayOfWeekShortToString(), dateTimeController.MonthShortToString(), day, year);
      }
      lv_obj_realign(label_date);

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepIcon);
  }

  if (!lv_obj_get_hidden(btnSet)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      lv_obj_set_hidden(btnSet, true);
      savedTick = 0;
    }
  }
}

bool WatchFaceDigital::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_get_hidden(btnClose)) {
    lv_obj_set_hidden(btnSet, false);
    savedTick = lv_tick_get();
    return true;
  }
  return false;
}

bool WatchFaceDigital::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnClose)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFaceDigital::CloseMenu() {
  settingsController.SaveSettings();
  HideSettingsMenuItems(true);
}

void WatchFaceDigital::HideSettingsMenuItems(bool visible) {
  lv_obj_set_hidden(btnClose, visible);
  lv_obj_set_hidden(settingsTitle, visible);
  for (int index = 0; index < optionsTotal; ++index) {
    lv_obj_set_hidden(cbOption[index], visible);
  }
  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}

void WatchFaceDigital::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    int index = 0;
    for (; index < optionsTotal; ++index) {
      if (cbOption[index] == object) {
        break;
      }
    }

    batteryPercentRemaining = batteryController.PercentRemaining();
    auto batteryPercent = batteryPercentRemaining.Get();

    if (index == 0) {
      if (settingsController.GetDWSBatteryPercentageStatus() == Controllers::Settings::BatteryPercentage::ON) {
        settingsController.SetDWSBatteryPercentageStatus(Controllers::Settings::BatteryPercentage::OFF);
        lv_checkbox_set_checked(cbOption[index], false);
        lv_label_set_text_static(batteryValue, "");
      } else {
        settingsController.SetDWSBatteryPercentageStatus(Controllers::Settings::BatteryPercentage::ON);
        lv_checkbox_set_checked(cbOption[index], true);
        lv_label_set_text_fmt(batteryValue, "%lu%%", batteryPercent);
      }
    };
    if (index == 1) {
      if (settingsController.GetDWSBatteryColorStatus() == Controllers::Settings::BatteryColor::ON) {
        settingsController.SetDWSBatteryColorStatus(Controllers::Settings::BatteryColor::OFF);
        lv_checkbox_set_checked(cbOption[index], false);
        lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BatteryIcon::GetDefaultBatteryColor(batteryPercent));
      } else {
        settingsController.SetDWSBatteryColorStatus(Controllers::Settings::BatteryColor::ON);
        lv_checkbox_set_checked(cbOption[index], true);
        lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BatteryIcon::GetBatteryColor(batteryPercent));
      }
    };
  }

  if (event == LV_EVENT_CLICKED) {
    if (object == btnClose) {
      CloseMenu();
    }
    if (object == btnSet) {
      lv_obj_set_hidden(btnSet, true);
      HideSettingsMenuItems(false);
      lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
    }
  }
}
