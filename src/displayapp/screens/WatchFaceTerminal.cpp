#include <date/date.h>
#include <lvgl/lvgl.h>
#include "displayapp/screens/WatchFaceTerminal.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

WatchFaceTerminal::WatchFaceTerminal(DisplayApp* app,
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
  settingsController.SetClockFace(3);

  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(batteryValue, true);
  lv_obj_align(batteryValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -20);

  connectState = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(connectState, true);
  lv_obj_align(connectState, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_LEFT_MID, 0, -100);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_date, true);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -40);

  label_prompt_1 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_prompt_1, true);
  lv_obj_align(label_prompt_1, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -80);

  label_ctf = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_ctf, true);
  lv_obj_align(label_ctf, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 60);
  lv_label_set_text_static(label_ctf, "[LVL ] #00FF00 .....");

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label_time, true);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -60);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(heartbeatValue, true);
  lv_obj_align(heartbeatValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 20);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(stepValue, true);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 40);
  

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceTerminal::~WatchFaceTerminal() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

static uint to_timestamp(uint hour, uint minute, uint second)
  {
    return (hour * 60 * 60) + (minute * 60) + second;
  }

void WatchFaceTerminal::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    lv_label_set_text_fmt(batteryValue, "[BATT] #00FF00 %d%%", batteryPercentRemaining.Get());
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValue, LV_LABEL_POS_LAST, " CHG");
    }
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    if(!bleRadioEnabled.Get()) {
      lv_label_set_text_static(connectState, "[BLE ] #0082fc DISABLED#");
    } else {
      if (bleState.Get()) {
        lv_label_set_text_static(connectState, "[BLE ]#00FF00  ON#");
      } else {
        lv_label_set_text_static(connectState, "[BLE ]#001000  OFF#");
      }
    }
  }

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (notificationState.Get()) {
      lv_label_set_text_static(notificationIcon, "You have mail.");
    } else {
      lv_label_set_text_static(notificationIcon, "");
    }
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
    uint8_t second = time.seconds().count();

    if (displayedHour != hour || displayedMinute != minute || displayedSecond != second) {
      displayedHour = hour;
      displayedMinute = minute;
      displayedSecond = second;

      bool is_leet = (to_timestamp(13, 37, 16) <= to_timestamp(hour, minute, second)) && 
                     (to_timestamp(hour, minute, second) <= to_timestamp(14, 0, 0));
      bool is_midnight = (to_timestamp(0, 0, 16) <= to_timestamp(hour, minute, second)) && 
                     (to_timestamp(hour, minute, second) <= to_timestamp(4, 0, 0));

      if (is_leet)
      {
          lv_label_set_text_static(label_prompt_1, "#00FFFF 1337##00FF00 @badge:~ $ now");
      }
      else if (is_midnight)
      {
        lv_label_set_text_static(label_prompt_1, "#FF0000 root##00FF00 @badge:~ ! now");
      }
      else
      {
          lv_label_set_text_static(label_prompt_1, "#00FF00 user@badge:~ $ now");
      }

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
        auto format = "[TIME] #00ff00 %02d:%02d:%02d %s#";
        if (is_leet)
        {
          format = "[T1M3] #00ffff %02X:%02X:%02X %s#";
        }
        else if (is_midnight)
        {
          format = "[T1M3] #ff0000 %02X:%02X:%02X %s#";
        }
        
        lv_label_set_text_fmt(label_time, format, hour, minute, second, ampmChar);
      } 
      else 
      {
        auto format = "[TIME] #00ff00 %02d:%02d:%02d#";
        if (is_leet)
        {
          format = "[T1M3] #00ffff %02X:%02X:%02X#";
        }
        else if (is_midnight)
        {
          format = "[T1M3] #ff0000 %02X:%02X:%02X#";
        }
        lv_label_set_text_fmt(label_time, format, hour, minute, second);
      }
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_fmt(label_date, "[DATE] #007f00 %02d_%02d_%04d#", char(day), char(month), short(year));

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
      lv_label_set_text_fmt(heartbeatValue, "[CPU ] #ee3311 %d bpm#", heartbeat.Get());
    } else {
      lv_label_set_text_static(heartbeatValue, "[CPU ] #ee3311 ---#");
    }
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "[STEP] #004000 %lu steps#", stepCount.Get());
  }
}
