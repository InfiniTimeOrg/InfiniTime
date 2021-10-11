#include "WatchFaceDigital.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "BatteryIcon.h"
#include "BleIcon.h"
#include "NotificationIcon.h"
#include "Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
using namespace Pinetime::Applications::Screens;

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
  settingsController.SetClockFace(0);

  batteryIcon = lv_label_create(lv_scr_act());
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, LV_ALIGN_TOP_RIGHT, 0, 0);

  batteryPlug = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(batteryPlug, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(batteryPlug, Symbols::plug);
  lv_obj_align_to(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(bleIcon, lv_color_hex(0x0000FF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(bleIcon, Symbols::bluetooth);
  lv_obj_align_to(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(notificationIcon, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, LV_ALIGN_TOP_LEFT, 0, 0);

  label_date = lv_label_create(lv_scr_act());
  lv_obj_align(label_date, LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_text_color(label_date, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DEFAULT);

  label_time = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(label_time, &jetbrains_mono_extrabold_compressed, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_align(label_time, LV_ALIGN_RIGHT_MID, 0, 0);

  label_time_ampm = lv_label_create(lv_scr_act());
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, LV_ALIGN_RIGHT_MID, -30, -55);

  backgroundLabel = lv_label_create(lv_scr_act());
  lv_obj_add_flag(backgroundLabel, LV_OBJ_FLAG_CLICKABLE);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  heartbeatIcon = lv_label_create(lv_scr_act());
  lv_label_set_text(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_text_color(heartbeatIcon, lv_color_hex(0xCE1B1B), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(heartbeatIcon, LV_ALIGN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(heartbeatValue, lv_color_hex(0xCE1B1B), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(heartbeatValue, "");
  lv_obj_align_to(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(stepValue, lv_color_hex(0x00FFE7), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(stepValue, "0");
  lv_obj_align(stepValue, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

  stepIcon = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(stepIcon, lv_color_hex(0x00FFE7), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(stepIcon, Symbols::shoe);
  lv_obj_align_to(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, this);
  Refresh();
}

WatchFaceDigital::~WatchFaceDigital() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceDigital::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_label_set_text(batteryPlug, BatteryIcon::GetPlugIcon(powerPresent.Get()));
  
    // These alignments cause lvgl to refresh the screen even if nothing has technically changed. Only align if needed.
    lv_obj_align(batteryIcon, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_align_to(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    if (batteryPercent == 100) {
      lv_obj_set_style_text_color(batteryIcon, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
      lv_obj_set_style_text_color(batteryIcon, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));
    
    // These alignments cause lvgl to refresh the screen even if nothing has technically changed. Only align if needed.
    lv_obj_align(batteryIcon, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_align_to(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    lv_label_set_text(bleIcon, BleIcon::GetIcon(bleState.Get()));
    
    // These alignments cause lvgl to refresh the screen even if nothing has technically changed. Only align if needed.
    lv_obj_align_to(bleIcon, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
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

    int hour = time.hours().count();
    auto minute = time.minutes().count();

    char minutesChar[3];
    sprintf(minutesChar, "%02d", static_cast<int>(minute));

    char hoursChar[3];
    char ampmChar[3];
    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
      sprintf(hoursChar, "%02d", hour);
    } else {
      if (hour == 0 && hour != 12) {
        hour = 12;
        sprintf(ampmChar, "AM");
      } else if (hour == 12 && hour != 0) {
        hour = 12;
        sprintf(ampmChar, "PM");
      } else if (hour < 12 && hour != 0) {
        sprintf(ampmChar, "AM");
      } else if (hour > 12 && hour != 0) {
        hour = hour - 12;
        sprintf(ampmChar, "PM");
      }
      sprintf(hoursChar, "%02d", hour);
    }

    if ((hoursChar[0] != displayedChar[0]) or (hoursChar[1] != displayedChar[1]) or (minutesChar[0] != displayedChar[2]) or
        (minutesChar[1] != displayedChar[3])) {
      displayedChar[0] = hoursChar[0];
      displayedChar[1] = hoursChar[1];
      displayedChar[2] = minutesChar[0];
      displayedChar[3] = minutesChar[1];

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        lv_label_set_text(label_time_ampm, ampmChar);
        if (hoursChar[0] == '0') {
          hoursChar[0] = ' ';
        }
      }

      lv_label_set_text_fmt(label_time, "%s:%s", hoursChar, minutesChar);

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        lv_obj_align(label_time, LV_ALIGN_RIGHT_MID, 0, 0);
      } else {
        lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
      }
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(label_date, "%s %d %s %d", dateTimeController.DayOfWeekShortToString(), day, dateTimeController.MonthShortToString(), year);
      } else {
        lv_label_set_text_fmt(label_date, "%s %s %d %d", dateTimeController.DayOfWeekShortToString(), dateTimeController.MonthShortToString(), day, year);
      }
      lv_obj_align(label_date, LV_ALIGN_CENTER, 0, 60);

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
      lv_obj_set_style_text_color(heartbeatIcon, lv_color_hex(0xCE1B1B), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_text_color(heartbeatIcon, lv_color_hex(0x1B1B1B), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_align(heartbeatIcon, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_align(heartbeatValue, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_align(stepValue, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_align_to(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }
}
