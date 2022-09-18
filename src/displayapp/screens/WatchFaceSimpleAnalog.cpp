#include "displayapp/screens/WatchFaceSimpleAnalog.h"
#include <cmath>
#include <lvgl/lvgl.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/NotificationIcon.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t HourLength = 60;
  constexpr int16_t MinuteLength = 95;
  constexpr int16_t SecondLength = 97;

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

WatchFaceSimpleAnalog::WatchFaceSimpleAnalog(Pinetime::Applications::DisplayApp* app,
                                             Controllers::DateTime& dateTimeController,
                                             Controllers::Battery& batteryController,
                                             Controllers::Ble& bleController,
                                             Controllers::NotificationManager& notificationManager,
                                             Controllers::Settings& settingsController,
                                             Controllers::HeartRateController& heartRateController,
                                             Controllers::MotionController& motionController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController} {

  sHour = 99;
  sMinute = 99;
  sSecond = 99;

  stepsMeter = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_range(stepsMeter, 0, 12);
  lv_linemeter_set_scale(stepsMeter, 360, 13);
  lv_linemeter_set_angle_offset(stepsMeter, 180);
  lv_obj_set_size(stepsMeter, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(stepsMeter, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_pad_all(stepsMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 0);

  batteryIcon.Create(lv_scr_act());
  lv_obj_align(batteryIcon.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  plugIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(plugIcon, Symbols::plug);
  lv_obj_set_style_local_text_color(plugIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_align(plugIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0082FC));
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, plugIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  // Date - Day / Week day

  label_date_day = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_date_day, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x90, 0x90, 0x90));
  lv_label_set_text_fmt(label_date_day, "%s %02i", dateTimeController.DayOfWeekShortToString(), dateTimeController.Day());
  lv_label_set_align(label_date_day, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_date_day, nullptr, LV_ALIGN_CENTER, 60, -2);

  minute_body_trace = lv_line_create(lv_scr_act(), nullptr);
  hour_body_trace = lv_line_create(lv_scr_act(), nullptr);
  minute_body = lv_line_create(lv_scr_act(), nullptr);
  hour_body = lv_line_create(lv_scr_act(), nullptr);
  second_body = lv_line_create(lv_scr_act(), nullptr);

  lv_style_init(&minute_line_style);
  lv_style_set_line_width(&minute_line_style, LV_STATE_DEFAULT, 9);
  lv_style_set_line_color(&minute_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&minute_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(minute_body, LV_LINE_PART_MAIN, &minute_line_style);

  lv_style_init(&minute_trace_line_style);
  lv_style_set_line_width(&minute_trace_line_style, LV_STATE_DEFAULT, 5);
  lv_style_set_line_color(&minute_trace_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&minute_trace_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(minute_body_trace, LV_LINE_PART_MAIN, &minute_trace_line_style);

  lv_style_init(&hour_line_style);
  lv_style_set_line_width(&hour_line_style, LV_STATE_DEFAULT, 9);
  lv_style_set_line_color(&hour_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&hour_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(hour_body, LV_LINE_PART_MAIN, &hour_line_style);

  lv_style_init(&hour_trace_line_style);
  lv_style_set_line_width(&hour_trace_line_style, LV_STATE_DEFAULT, 5);
  lv_style_set_line_color(&hour_trace_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&hour_trace_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(hour_body_trace, LV_LINE_PART_MAIN, &hour_trace_line_style);

  lv_style_init(&second_line_style);
  lv_style_set_line_width(&second_line_style, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&second_line_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_style_set_line_rounded(&second_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(second_body, LV_LINE_PART_MAIN, &second_line_style);

  dot = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(dot, 10, 10);
  lv_obj_align(dot, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_radius(dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(dot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  Refresh();
}

WatchFaceSimpleAnalog::~WatchFaceSimpleAnalog() {
  lv_task_del(taskRefresh);

  lv_style_reset(&hour_line_style);
  lv_style_reset(&hour_trace_line_style);
  lv_style_reset(&minute_line_style);
  lv_style_reset(&minute_trace_line_style);
  lv_style_reset(&second_line_style);

  lv_obj_clean(lv_scr_act());
}

void WatchFaceSimpleAnalog::UpdateClock() {
  uint8_t hour = dateTimeController.Hours();
  uint8_t minute = dateTimeController.Minutes();
  uint8_t second = dateTimeController.Seconds();

  if (sMinute != minute) {
    auto const angle = minute * 6;

    minute_point[0] = CoordinateRelocate(16, angle);
    minute_point[1] = CoordinateRelocate(MinuteLength, angle);

    minute_trace_point[0] = CoordinateRelocate(0, angle);
    minute_trace_point[1] = minute_point[0];

    lv_line_set_points(minute_body, minute_point, 2);
    lv_line_set_points(minute_body_trace, minute_trace_point, 2);
  }

  if (sHour != hour || sMinute != minute) {
    sHour = hour;
    sMinute = minute;
    auto const angle = (hour * 30 + minute / 2);

    hour_point[0] = CoordinateRelocate(16, angle);
    hour_point[1] = CoordinateRelocate(HourLength, angle);

    hour_trace_point[0] = CoordinateRelocate(0, angle);
    hour_trace_point[1] = hour_point[1];

    lv_line_set_points(hour_body, hour_point, 2);
    lv_line_set_points(hour_body_trace, hour_trace_point, 2);
  }

  if (sSecond != second) {
    sSecond = second;
    auto const angle = second * 6;

    second_point[0] = CoordinateRelocate(-20, angle);
    second_point[1] = CoordinateRelocate(SecondLength, angle);

    lv_line_set_points(second_body, second_point, 2);
  }
}

void WatchFaceSimpleAnalog::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}

void WatchFaceSimpleAnalog::Refresh() {
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
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }
  lv_obj_realign(plugIcon);
  lv_obj_realign(bleIcon);

  notificationState = notificationManager.AreNewNotificationsAvailable();

  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = dateTimeController.CurrentDateTime();

  if (currentDateTime.IsUpdated()) {
    Pinetime::Controllers::DateTime::Months month = dateTimeController.Month();
    uint8_t day = dateTimeController.Day();
    Pinetime::Controllers::DateTime::Days dayOfWeek = dateTimeController.DayOfWeek();

    UpdateClock();

    if ((month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_fmt(label_date_day, "%s %02i", dateTimeController.DayOfWeekShortToString(), day);

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
    uint32_t goal = settingsController.GetStepsGoal();
    uint32_t steps = stepCount.Get();
    int32_t steps_value;
    if (steps > goal)
      steps_value = 12;
    else
      steps_value = ((float) steps / goal) * 12;

    lv_linemeter_set_value(stepsMeter, steps_value);
  }
}
