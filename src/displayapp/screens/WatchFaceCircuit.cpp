#include "displayapp/screens/WatchFaceCircuit.h"

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

LV_IMG_DECLARE(bg_circuit);

using namespace Pinetime::Applications::Screens;

WatchFaceCircuit::WatchFaceCircuit(DisplayApp* app,
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

  lv_obj_t* bg_circuit_img = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(bg_circuit_img, &bg_circuit);
  lv_obj_align(bg_circuit_img, nullptr, LV_ALIGN_CENTER, 0, 0);

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryPlug, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));
  lv_label_set_text(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0000FF));
  lv_label_set_text(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 30, 46);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

  label_hour = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(label_hour, "00");
  lv_obj_set_style_local_text_font(label_hour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_pos(label_hour, 115, 55);

  label_minute = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(label_minute, "00");
  lv_obj_set_style_local_text_font(label_minute, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_pos(label_minute, 115, 145);

//   label_v_out = lv_label_create(lv_scr_act(), nullptr);
//   lv_obj_set_pos(label_v_out, 180, 110);

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -30, -55);

//   backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
//   lv_obj_set_click(backgroundLabel, true);
//   lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
//   lv_obj_set_size(backgroundLabel, 240, 240);
//   lv_obj_set_pos(backgroundLabel, 0, 0);
//   lv_label_set_text(backgroundLabel, "");

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_label_set_text(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceCircuit::~WatchFaceCircuit() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceCircuit::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_label_set_text(batteryPlug, BatteryIcon::GetPlugIcon(powerPresent.Get()));
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    if (batteryPercent == 100) {
      lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    } else {
      lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    }
    lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    lv_label_set_text(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

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

    auto month = static_cast<Pinetime::Controllers::DateTime::Months>(static_cast<unsigned>(yearMonthDay.month()));
    auto day = static_cast<unsigned>(yearMonthDay.day());

    int hour = time.hours().count();
    auto minute = time.minutes().count();

	// double v_in = 0;
	// double v_out = 0;

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

      lv_label_set_text_fmt(label_hour, "%sOΩ", hoursChar);

	  lv_label_set_text_fmt(label_minute, "%sOΩ", minutesChar);



    //   if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    //     lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);
    //   } else {
    //     lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
    //   }
    }

    if ((month != currentMonth) || (day != currentDay)) {
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(label_date, "%d\n%d", day, month);
		// v_in = 0; //day/month;
      } else {
        lv_label_set_text_fmt(label_date, "%d\n%d", month, day);
		// v_in = 0; //month/day;
      }
      lv_obj_set_pos(label_date, 60, 80);

      currentMonth = month;
      currentDay = day;
    }

	  //Voltage details here
	//   v_out = 0; //(v_in)*(hours/(minute+hours));
    //   lv_label_set_text_fmt(label_v_out, "%0.2f V", v_out);
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

    lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }
}
