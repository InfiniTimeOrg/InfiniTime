#include "WatchFaceDigital.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "BatteryIcon.h"
#include "BleIcon.h"
#include "NotificationIcon.h"
#include "Symbols.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::DateTime;

WatchFaceDigital::WatchFaceDigital(DisplayApp* app,
                                   Controllers::DateTimeController const& dateTimeController,
                                   Controllers::Battery const& batteryController,
                                   Controllers::Ble const& bleController,
                                   Controllers::NotificationManager const& notificationManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController const& heartRateController,
                                   Controllers::MotionController const& motionController)
  : WatchFaceBase{Pinetime::Controllers::Settings::ClockFace::Digital,
      app,
      settingsController,
      dateTimeController,
      batteryController,
      bleController,
      notificationManager,
      heartRateController,
      motionController} {

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 2);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryPlug, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));
  lv_label_set_text(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0000FF));
  lv_label_set_text(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 10, 0);

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
  lv_label_set_text(backgroundLabel, "");

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -2);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_label_set_text(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -2);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
}

WatchFaceDigital::~WatchFaceDigital() {
  lv_obj_clean(lv_scr_act());
}

bool WatchFaceDigital::Refresh() {
  auto const& battery = GetUpdatedBattery();
  if (battery.IsUpdated()) {
    auto const& b = battery.Get();
    lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(b.percentRemaining));
    auto const isCharging = b.charging || b.powerPresent;
    lv_label_set_text(batteryPlug, BatteryIcon::GetPlugIcon(isCharging));
  }

  auto const& ble = GetUpdatedBle();
  if (ble.IsUpdated()) {
    lv_label_set_text(bleIcon, BleIcon::GetIcon(ble.Get().connected));
  }
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 5);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  auto const& notifications = GetUpdatedNotifications();
  if (notifications.IsUpdated()) {
    auto const icon = NotificationIcon::GetIcon(notifications.Get().newNotificationsAvailable);
    lv_label_set_text(notificationIcon, icon);
  }

  auto const clockType = GetClockType();
  auto const& time = GetUpdatedTime();
  if (time.IsUpdated()) {
    auto const& t = time.Get();
    hour = t.hour;
    minute = t.minute;

    if (hour.IsUpdated() || minute.IsUpdated()) {
      auto hourTemp = hour.Get();

      if (clockType == Controllers::Settings::ClockType::H12) {
        lv_label_set_text(label_time_ampm, hourTemp < 12 ? "AM" : "PM");

        if (hourTemp == 0) {
          hourTemp = 12;
        } else if (hourTemp > 12) {
          hourTemp -= 12;
        }

        lv_label_set_text_fmt(label_time, "%2" PRIu8 ":%02" PRIu8, hourTemp, minute.Get());
        lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);
      } else if (clockType == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(label_time, "%02" PRIu8 ":%02" PRIu8, hourTemp, minute.Get());
        lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
      }
    }
  }

  auto const& date = GetUpdatedDate();
  if (date.IsUpdated()) {
    auto const& d = date.Get();

    lv_label_set_text_fmt(label_date, "%s %" PRIu8 " %s %" PRIu16, DayOfWeekShortToString(d.dayOfWeek),
                          d.day, MonthShortToString(d.month), d.year);
    lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  }

  auto const& heartRate = GetUpdatedHeartRate();
  if (heartRate.IsUpdated()) {
    auto const& hr = heartRate.Get();

    if (hr.running) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
      lv_label_set_text_fmt(heartbeatValue, "%d", hr.rate);
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 5, -2);
    lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  }

  auto const& motion = GetUpdatedMotion();
  if (motion.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", motion.Get().stepCount);
    lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -2);
    lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  return running;
}
