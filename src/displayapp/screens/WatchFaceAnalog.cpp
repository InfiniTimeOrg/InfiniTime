#include "displayapp/screens/WatchFaceAnalog.h"
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
  constexpr int16_t HourLength = 70;
  constexpr int16_t MinuteLength = 90;
  constexpr int16_t SecondLength = 110;

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

  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFaceAnalog*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }

}

WatchFaceAnalog::WatchFaceAnalog(Controllers::DateTime& dateTimeController,
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
  sSecond = 99;

  minor_scales = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(minor_scales, 300, 51);
  lv_linemeter_set_angle_offset(minor_scales, 180);
  lv_obj_set_size(minor_scales, 240, 240);
  lv_obj_align(minor_scales, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_line_width(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_scale_end_color(minor_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  major_scales = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(major_scales, 300, 11);
  lv_linemeter_set_angle_offset(major_scales, 180);
  lv_obj_set_size(major_scales, 240, 240);
  lv_obj_align(major_scales, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 6);
  lv_obj_set_style_local_scale_end_line_width(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_color(major_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  large_scales = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(large_scales, 180, 3);
  lv_linemeter_set_angle_offset(large_scales, 180);
  lv_obj_set_size(large_scales, 240, 240);
  lv_obj_align(large_scales, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(large_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(large_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_scale_end_line_width(large_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_color(large_scales, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  twelve = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(twelve, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(twelve, "12");
  if (settingsController.GetA24HourMode() == Pinetime::Controllers::Settings::A24HourMode::On) {
    lv_obj_set_pos(twelve, 110, 210);
  } else {
    lv_obj_set_pos(twelve, 110, 10);
  }
  lv_obj_set_style_local_text_color(twelve, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  zero = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(zero, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(zero, "00");
  lv_obj_set_pos(zero, 110, 10);
  lv_obj_set_style_local_text_color(zero, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  if (settingsController.GetA24HourMode() == Pinetime::Controllers::Settings::A24HourMode::On) {
    lv_obj_set_hidden(zero, false);
  } else {
    lv_obj_set_hidden(zero, true);
  }

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
  lv_obj_set_style_local_text_color(label_date_day, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
  lv_label_set_text_fmt(label_date_day, "%s\n%02i", dateTimeController.DayOfWeekShortToString(), dateTimeController.Day());
  lv_label_set_align(label_date_day, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_date_day, nullptr, LV_ALIGN_CENTER, 50, 0);

  minute_body = lv_line_create(lv_scr_act(), nullptr);
  /* minute_body_trace = lv_line_create(lv_scr_act(), nullptr); */
  hour_body = lv_line_create(lv_scr_act(), nullptr);
  /* hour_body_trace = lv_line_create(lv_scr_act(), nullptr); */
  second_body = lv_line_create(lv_scr_act(), nullptr);

  lv_style_init(&second_line_style);
  lv_style_set_line_width(&second_line_style, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&second_line_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_style_set_line_rounded(&second_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(second_body, LV_LINE_PART_MAIN, &second_line_style);
  if (settingsController.GetASecondHand() == Pinetime::Controllers::Settings::ASecondHand::On) {
    lv_obj_set_hidden(second_body, false);
  } else {
    lv_obj_set_hidden(second_body, true);
  }

  lv_style_init(&minute_line_style);
  lv_style_set_line_width(&minute_line_style, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&minute_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&minute_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(minute_body, LV_LINE_PART_MAIN, &minute_line_style);

  /* lv_style_init(&minute_line_style_trace); */
  /* lv_style_set_line_width(&minute_line_style_trace, LV_STATE_DEFAULT, 3); */
  /* lv_style_set_line_color(&minute_line_style_trace, LV_STATE_DEFAULT, LV_COLOR_WHITE); */
  /* lv_style_set_line_rounded(&minute_line_style_trace, LV_STATE_DEFAULT, false); */
  /* lv_obj_add_style(minute_body_trace, LV_LINE_PART_MAIN, &minute_line_style_trace); */

  lv_style_init(&hour_line_style);
  lv_style_set_line_width(&hour_line_style, LV_STATE_DEFAULT, 5);
  lv_style_set_line_color(&hour_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&hour_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(hour_body, LV_LINE_PART_MAIN, &hour_line_style);

  /* lv_style_init(&hour_line_style_trace); */
  /* lv_style_set_line_width(&hour_line_style_trace, LV_STATE_DEFAULT, 3); */
  /* lv_style_set_line_color(&hour_line_style_trace, LV_STATE_DEFAULT, LV_COLOR_WHITE); */
  /* lv_style_set_line_rounded(&hour_line_style_trace, LV_STATE_DEFAULT, false); */
  /* lv_obj_add_style(hour_body_trace, LV_LINE_PART_MAIN, &hour_line_style_trace); */

  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblClose = lv_label_create(btnClose, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  btn24HourMode = lv_btn_create(lv_scr_act(), nullptr);
  btn24HourMode->user_data = this;
  lv_obj_set_size(btn24HourMode, 160, 60);
  lv_obj_align(btn24HourMode, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);
  lv_obj_set_style_local_bg_opa(btn24HourMode, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lbl24HourMode = lv_label_create(btn24HourMode, nullptr);
  lv_label_set_text_static(lbl24HourMode, "Toggle 24h");
  lv_obj_set_event_cb(btn24HourMode, event_handler);
  lv_obj_set_hidden(btn24HourMode, true);

  btnSecondHand = lv_btn_create(lv_scr_act(), nullptr);
  btnSecondHand->user_data = this;
  lv_obj_set_size(btnSecondHand, 160, 60);
  lv_obj_align(btnSecondHand, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_local_bg_opa(btnSecondHand, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblSecondHand = lv_label_create(btnSecondHand, nullptr);
  lv_label_set_text_static(lblSecondHand, "Seconds");
  lv_obj_set_event_cb(btnSecondHand, event_handler);
  lv_obj_set_hidden(btnSecondHand, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  Refresh();
}

WatchFaceAnalog::~WatchFaceAnalog() {
  lv_task_del(taskRefresh);

  lv_style_reset(&hour_line_style);
  /* lv_style_reset(&hour_line_style_trace); */
  lv_style_reset(&minute_line_style);
  /* lv_style_reset(&minute_line_style_trace); */
  lv_style_reset(&second_line_style);

  lv_obj_clean(lv_scr_act());
}

/* Begin Settings */
bool WatchFaceAnalog::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_get_hidden(btnClose)) {
    lv_obj_set_hidden(btnClose, false);
    lv_obj_set_hidden(btn24HourMode, false);
    lv_obj_set_hidden(btnSecondHand, false);
    savedTick = lv_tick_get();
    return true;
  }
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && (lv_obj_get_hidden(btnClose) == false)) {
    return true;
  }
  return false;
}

void WatchFaceAnalog::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_set_hidden(btnClose, true);
  lv_obj_set_hidden(btn24HourMode, true);
  lv_obj_set_hidden(btnSecondHand, true);
}

bool WatchFaceAnalog::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnClose)) {
    CloseMenu();
    return true;
  }
  return false;
}
/* End Settings */

void WatchFaceAnalog::UpdateClock() {
  uint8_t hour = dateTimeController.Hours();
  uint8_t minute = dateTimeController.Minutes();
  uint8_t second = dateTimeController.Seconds();

  if (sMinute != minute) {
    auto const angle = minute * 6;
    minute_point[0] = CoordinateRelocate(-15, angle);
    minute_point[1] = CoordinateRelocate(MinuteLength, angle);

    /* minute_point_trace[0] = CoordinateRelocate(5, angle); */
    /* minute_point_trace[1] = CoordinateRelocate(31, angle); */

    lv_line_set_points(minute_body, minute_point, 2);
    /* lv_line_set_points(minute_body_trace, minute_point_trace, 2); */
  }

  if (sHour != hour || sMinute != minute) {
    sHour = hour;
    sMinute = minute;
    auto const angle = (hour * 30 + minute / 2);

    hour_point[0] = CoordinateRelocate(-10, angle);
    hour_point[1] = CoordinateRelocate(HourLength, angle);

    /* hour_point_trace[0] = CoordinateRelocate(5, angle); */
    /* hour_point_trace[1] = CoordinateRelocate(31, angle); */

    lv_line_set_points(hour_body, hour_point, 2);
    /* lv_line_set_points(hour_body_trace, hour_point_trace, 2); */
  }

  if (sSecond != second) {
    sSecond = second;
    auto const angle = second * 6;

    second_point[0] = CoordinateRelocate(-20, angle);
    second_point[1] = CoordinateRelocate(SecondLength, angle);
    lv_line_set_points(second_body, second_point, 2);
  }
}

void WatchFaceAnalog::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}

void WatchFaceAnalog::Refresh() {
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

void WatchFaceAnalog::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (object == btnClose) {
      CloseMenu();
    }
    if (object == btn24HourMode) {
      if (lv_obj_get_hidden(zero)) {
        // set mode to 24 hours
        lv_obj_set_pos(twelve, 110, 210);
        lv_obj_set_hidden(zero, false);
        settingsController.SetA24HourMode(Controllers::Settings::A24HourMode::On);
      } else {
        // set mode to 12 hours
        lv_obj_set_pos(twelve, 110, 10);
        lv_obj_set_hidden(zero, true);
        settingsController.SetA24HourMode(Controllers::Settings::A24HourMode::Off);
      }
    }
    if (object == btnSecondHand) {
      if (lv_obj_get_hidden(second_body)) {
        lv_obj_set_hidden(second_body, false);
      } else {
        lv_obj_set_hidden(second_body, true);
      }
    }
  }
}
