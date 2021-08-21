#include <libs/lvgl/lvgl.h>
#include "WatchFaceAnalog.h"
#include "BatteryIcon.h"
#include "Symbols.h"
#include "NotificationIcon.h"
#include "components/settings/Settings.h"

LV_IMG_DECLARE(bg_clock);

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::DateTime;

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

}

WatchFaceAnalog::WatchFaceAnalog(Pinetime::Applications::DisplayApp* app,
                                 Controllers::DateTimeController const& dateTimeController,
                                 Controllers::Battery const& batteryController,
                                 Controllers::Ble const& bleController,
                                 Controllers::NotificationManager const& notificationManager,
                                 Controllers::Settings& settingsController,
                                 Controllers::HeartRateController const& heartRateController,
                                 Controllers::MotionController const& motionController)
  : WatchFaceBase {Pinetime::Controllers::Settings::ClockFace::Analog,
                   app,
                   settingsController,
                   dateTimeController,
                   batteryController,
                   bleController,
                   notificationManager,
                   heartRateController,
                   motionController} {

  lv_obj_t* bg_clock_img = lv_img_create(lv_scr_act(), NULL);
  lv_img_set_src(bg_clock_img, &bg_clock);
  lv_obj_align(bg_clock_img, NULL, LV_ALIGN_CENTER, 0, 0);

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(batteryIcon, Symbols::batteryHalf);
  lv_obj_align(batteryIcon, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  notificationIcon = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  // Date - Day / Week day
  auto const& date = GetUpdatedDate().Get();
  label_date_day = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(label_date_day, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xf0a500));
  lv_label_set_text_fmt(label_date_day, "%s\n%02i", DayOfWeekShortToString(date.dayOfWeek), date.day);
  lv_label_set_align(label_date_day, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_date_day, NULL, LV_ALIGN_CENTER, 50, 0);

  minute_body = lv_line_create(lv_scr_act(), NULL);
  minute_body_trace = lv_line_create(lv_scr_act(), NULL);
  hour_body = lv_line_create(lv_scr_act(), NULL);
  hour_body_trace = lv_line_create(lv_scr_act(), NULL);
  second_body = lv_line_create(lv_scr_act(), NULL);

  lv_style_init(&second_line_style);
  lv_style_set_line_width(&second_line_style, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&second_line_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_style_set_line_rounded(&second_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(second_body, LV_LINE_PART_MAIN, &second_line_style);

  lv_style_init(&minute_line_style);
  lv_style_set_line_width(&minute_line_style, LV_STATE_DEFAULT, 7);
  lv_style_set_line_color(&minute_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&minute_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(minute_body, LV_LINE_PART_MAIN, &minute_line_style);

  lv_style_init(&minute_line_style_trace);
  lv_style_set_line_width(&minute_line_style_trace, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&minute_line_style_trace, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&minute_line_style_trace, LV_STATE_DEFAULT, false);
  lv_obj_add_style(minute_body_trace, LV_LINE_PART_MAIN, &minute_line_style_trace);

  lv_style_init(&hour_line_style);
  lv_style_set_line_width(&hour_line_style, LV_STATE_DEFAULT, 7);
  lv_style_set_line_color(&hour_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&hour_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(hour_body, LV_LINE_PART_MAIN, &hour_line_style);

  lv_style_init(&hour_line_style_trace);
  lv_style_set_line_width(&hour_line_style_trace, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&hour_line_style_trace, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&hour_line_style_trace, LV_STATE_DEFAULT, false);
  lv_obj_add_style(hour_body_trace, LV_LINE_PART_MAIN, &hour_line_style_trace);

  UpdateClock();
}

WatchFaceAnalog::~WatchFaceAnalog() {
  lv_style_reset(&hour_line_style);
  lv_style_reset(&hour_line_style_trace);
  lv_style_reset(&minute_line_style);
  lv_style_reset(&minute_line_style_trace);
  lv_style_reset(&second_line_style);

  lv_obj_clean(lv_scr_act());
}

void WatchFaceAnalog::UpdateClock() {
  bool const minute_changed = minute.IsUpdated();
  if (minute_changed) {
    auto const angle = minute.Get() * 6;
    minute_point[0] = CoordinateRelocate(30, angle);
    minute_point[1] = CoordinateRelocate(MinuteLength, angle);

    minute_point_trace[0] = CoordinateRelocate(5, angle);
    minute_point_trace[1] = CoordinateRelocate(31, angle);

    lv_line_set_points(minute_body, minute_point, 2);
    lv_line_set_points(minute_body_trace, minute_point_trace, 2);
  }

  if (hour.IsUpdated() || minute_changed) {
    auto const angle = (hour.Get() * 30 + minute.Get() / 2);

    hour_point[0] = CoordinateRelocate(30, angle);
    hour_point[1] = CoordinateRelocate(HourLength, angle);

    hour_point_trace[0] = CoordinateRelocate(5, angle);
    hour_point_trace[1] = CoordinateRelocate(31, angle);

    lv_line_set_points(hour_body, hour_point, 2);
    lv_line_set_points(hour_body_trace, hour_point_trace, 2);
  }

  if (second.IsUpdated()) {
    auto const angle = second.Get() * 6;

    second_point[0] = CoordinateRelocate(-20, angle);
    second_point[1] = CoordinateRelocate(SecondLength, angle);

    lv_line_set_points(second_body, second_point, 2);
  }
}

bool WatchFaceAnalog::Refresh() {
  auto const& battery = GetUpdatedBattery();
  if (battery.IsUpdated()) {
    auto const icon = BatteryIcon::GetBatteryIcon(battery.Get().percentRemaining);
    lv_label_set_text_static(batteryIcon, icon);
  }

  auto const& notifications = GetUpdatedNotifications();
  if (notifications.IsUpdated()) {
    auto const icon = NotificationIcon::GetIcon(notifications.Get().newNotificationsAvailable);
    lv_label_set_text_static(notificationIcon, icon);
  }

  auto const& time = GetUpdatedTime();
  if (time.IsUpdated()) {
    auto const& t = time.Get();
    hour = t.hour;
    minute = t.minute;
    second = t.second;

    UpdateClock();
  }

  auto const& date = GetUpdatedDate();
  if (date.IsUpdated()) {
    auto const& d = date.Get();
    lv_label_set_text_fmt(label_date_day, "%s\n%02i", DayOfWeekShortToString(d.dayOfWeek), d.day);
  }

  return true;
}
