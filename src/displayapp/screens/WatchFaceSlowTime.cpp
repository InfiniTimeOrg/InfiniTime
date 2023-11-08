#include "displayapp/screens/WatchFaceSlowTime.h"
#include <cmath>
#include <lvgl/lvgl.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/NotificationIcon.h"
#include "components/settings/Settings.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t HourLength = 50;
  constexpr int16_t MinuteLength = 80;

  // sin(90) = 1 so the value of _lv_trigo_sin(90) is the scaling factor
  const auto LV_TRIG_SCALE = _lv_trigo_sin(90);

  int16_t Cosine(int16_t angle) {
    return _lv_trigo_sin(angle + 90);
  }

  int16_t Sine(int16_t angle) {
    return _lv_trigo_sin(angle);
  }

  int16_t CoordinateXRelocate(int16_t x) {
    return (x + LV_HOR_RES / 2);
  }

  int16_t CoordinateYRelocate(int16_t y) {
    return std::abs(y - LV_HOR_RES / 2);
  }

  lv_point_t CoordinateRelocate(int16_t radius, int16_t angle) {
    return lv_point_t {.x = CoordinateXRelocate(radius * static_cast<int32_t>(Sine(angle)) / LV_TRIG_SCALE),
                       .y = CoordinateYRelocate(radius * static_cast<int32_t>(Cosine(angle)) / LV_TRIG_SCALE)};
  }

}

WatchFaceSlowTime::WatchFaceSlowTime(Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 Controllers::NotificationManager& notificationManager,
                                 Controllers::Settings& settingsController)
  : currentDateTime {{}},
    batteryIcon(true),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController} {

  sHour = 99;
  sMinute = 99;

  minor_scales = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(minor_scales, 360, 61);
  lv_linemeter_set_angle_offset(minor_scales, 180);
  lv_obj_set_size(minor_scales, 240, 240);
  lv_obj_align(minor_scales, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_line_width(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_scale_end_color(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  major_scales= lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(major_scales, 360, 13);
  lv_linemeter_set_angle_offset(major_scales, 30);
  lv_obj_set_size(major_scales, 240, 240);
  lv_obj_align(major_scales, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_line_width(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_scale_end_color(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  large_scales= lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(large_scales, 360, 13);
  lv_linemeter_set_angle_offset(large_scales, 15);
  lv_obj_set_size(large_scales, 220, 220);
  lv_obj_align(large_scales, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(large_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(large_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_scale_end_line_width(large_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_scale_end_color(large_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  two = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(two, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(two, "2");
  lv_obj_set_pos(two, 160, 30);
  lv_obj_set_style_local_text_color(two, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  four = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(four, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(four, "4");
  lv_obj_set_pos(four, 192, 60);
  lv_obj_set_style_local_text_color(four, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  six = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(six, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(six, "6");
  lv_obj_set_pos(six, 206, 107);
  lv_obj_set_style_local_text_color(six, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  eight = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(eight, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(eight, "8");
  lv_obj_set_pos(eight, 192, 154);
  lv_obj_set_style_local_text_color(eight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  ten = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(ten, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(ten, "10");
  lv_obj_set_pos(ten, 153, 184);
  lv_obj_set_style_local_text_color(ten, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  twelve = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(twelve, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(twelve, "12");
  lv_obj_set_pos(twelve, 107, 198);
  lv_obj_set_style_local_text_color(twelve, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  fourteen = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(fourteen, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(fourteen, "14");
  lv_obj_set_pos(fourteen, 62, 184);
  lv_obj_set_style_local_text_color(fourteen, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  sixteen = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(sixteen, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(sixteen, "16");
  lv_obj_set_pos(sixteen, 32, 152);
  lv_obj_set_style_local_text_color(sixteen, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  eighteen = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(eighteen, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(eighteen, "18");
  lv_obj_set_pos(eighteen, 20, 107);
  lv_obj_set_style_local_text_color(eighteen, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  twenty = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(twenty, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(twenty, "20");
  lv_obj_set_pos(twenty, 32, 62);
  lv_obj_set_style_local_text_color(twenty, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  twentytwo = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(twentytwo, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(twentytwo, "22");
  lv_obj_set_pos(twentytwo, 62, 30);
  lv_obj_set_style_local_text_color(twentytwo, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  twentyfour = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(twentyfour, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(twentyfour, "24");
  lv_obj_set_pos(twentyfour, 107, 17);
  lv_obj_set_style_local_text_color(twentyfour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  batteryIcon.Create(lv_scr_act());
  lv_obj_align(batteryIcon.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  plugIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(plugIcon, Symbols::plug);
  lv_obj_align(plugIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bleIcon, "");
  lv_obj_align(bleIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, -30, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  // Date - Day / Week day

  label_date_day = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_date_day, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAROON);
  lv_label_set_text_fmt(label_date_day, "%s\n%02i", dateTimeController.DayOfWeekShortToString(), dateTimeController.Day());
  lv_label_set_align(label_date_day, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_date_day, nullptr, LV_ALIGN_CENTER, 50, 0);

  minute_body = lv_line_create(lv_scr_act(), nullptr);
  hour_body = lv_line_create(lv_scr_act(), nullptr);

  lv_style_init(&minute_line_style);
  lv_style_set_line_width(&minute_line_style, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&minute_line_style, LV_STATE_DEFAULT, LV_COLOR_TEAL);
  lv_style_set_line_rounded(&minute_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(minute_body, LV_LINE_PART_MAIN, &minute_line_style);

  lv_style_init(&hour_line_style);
  lv_style_set_line_width(&hour_line_style, LV_STATE_DEFAULT, 5);
  lv_style_set_line_color(&hour_line_style, LV_STATE_DEFAULT, LV_COLOR_TEAL);
  lv_style_set_line_rounded(&hour_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(hour_body, LV_LINE_PART_MAIN, &hour_line_style);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  Refresh();
}

WatchFaceSlowTime::~WatchFaceSlowTime() {
  lv_task_del(taskRefresh);

  lv_style_reset(&hour_line_style);
  lv_style_reset(&minute_line_style);

  lv_obj_clean(lv_scr_act());
}

void WatchFaceSlowTime::UpdateClock() {
  uint8_t hour = dateTimeController.Hours();
  uint8_t minute = dateTimeController.Minutes();

  if (sMinute != minute) {
    auto const angle = minute * 6;
    minute_point[0] = CoordinateRelocate(-15, angle);
    minute_point[1] = CoordinateRelocate(MinuteLength, angle);

    lv_line_set_points(minute_body, minute_point, 2);
  }

  if (sHour != hour || sMinute != minute) {
    sHour = hour;
    sMinute = minute;
    auto const angle = (hour * 15 + minute / 4);

    hour_point[0] = CoordinateRelocate(-15, angle);
    hour_point[1] = CoordinateRelocate(HourLength, angle);

    lv_line_set_points(hour_body, hour_point, 2);
  }
}

void WatchFaceSlowTime::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}

void WatchFaceSlowTime::Refresh() {
  isCharging = batteryController.IsCharging();
  if (isCharging.IsUpdated()) {
    if (isCharging.Get()) {
      lv_obj_set_hidden(batteryIcon.GetObject(), true);
      lv_obj_set_hidden(plugIcon, false);
    } else {
      lv_obj_set_hidden(batteryIcon.GetObject(), false);
      lv_obj_set_hidden(plugIcon, true);
      SetBatteryIcon();
    }
  }
  if (!isCharging.Get()) {
    batteryPercentRemaining = batteryController.PercentRemaining();
    if (batteryPercentRemaining.IsUpdated()) {
      SetBatteryIcon();
    }
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    if (bleState.Get()) {
      lv_label_set_text_static(bleIcon, Symbols::bluetooth);
    } else {
      lv_label_set_text_static(bleIcon, "");
    }
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();

  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    UpdateClock();

    currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      lv_label_set_text_fmt(label_date_day, "%s\n%02i", dateTimeController.DayOfWeekShortToString(), dateTimeController.Day());
    }
  }
}
