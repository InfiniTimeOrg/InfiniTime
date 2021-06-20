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

constexpr auto HOUR_LENGTH = 70;
constexpr auto MINUTE_LENGTH = 90;
constexpr auto SECOND_LENGTH = 110;

// sin(90) = 1 so the value of _lv_trigo_sin(90) is the scaling factor
const auto LV_TRIG_SCALE = _lv_trigo_sin(90);

int16_t cosine(int16_t angle) {
  return _lv_trigo_sin(angle + 90);
}

int16_t sine(int16_t angle) {
  return _lv_trigo_sin(angle);
}

int16_t coordinate_x_relocate(int16_t x) {
  return (x + LV_HOR_RES / 2);
}

int16_t coordinate_y_relocate(int16_t y) {
  return std::abs(y - LV_HOR_RES / 2);
}

lv_point_t coordinate_relocate(int16_t radius, int16_t angle) {
  return lv_point_t{
    .x = coordinate_x_relocate(radius * static_cast<int32_t>(sine(angle)) / LV_TRIG_SCALE),
    .y = coordinate_y_relocate(radius * static_cast<int32_t>(cosine(angle)) / LV_TRIG_SCALE)
  };
}

} // namespace

WatchFaceAnalog::WatchFaceAnalog(Pinetime::Applications::DisplayApp* app,
                                 Controllers::DateTimeController const& dateTimeController,
                                 Controllers::Battery const& batteryController,
                                 Controllers::Ble const& bleController,
                                 Controllers::NotificationManager const& notificationManager,
                                 Controllers::Settings& settingsController)
  : WatchFaceBase{Pinetime::Controllers::Settings::ClockFace::Analog,
      app,
      settingsController,
      dateTimeController,
      batteryController,
      bleController,
      notificationManager} {

  lv_obj_t* bg_clock_img = lv_img_create(lv_scr_act(), NULL);
  lv_img_set_src(bg_clock_img, &bg_clock);
  lv_obj_align(bg_clock_img, NULL, LV_ALIGN_CENTER, 0, 0);

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryIcon, Symbols::batteryHalf);
  lv_obj_align(batteryIcon, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -8, -4);

  notificationIcon = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 8, -4);

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
    minute_point[0] = coordinate_relocate(30, angle);
    minute_point[1] = coordinate_relocate(MINUTE_LENGTH, angle);

    minute_point_trace[0] = coordinate_relocate(5, angle);
    minute_point_trace[1] = coordinate_relocate(31, angle);

    lv_line_set_points(minute_body, minute_point, 2);
    lv_line_set_points(minute_body_trace, minute_point_trace, 2);
  }

  if (hour.IsUpdated() || minute_changed) {
    auto const angle = (hour.Get() * 30 + minute.Get() / 2);

    hour_point[0] = coordinate_relocate(30, angle);
    hour_point[1] = coordinate_relocate(HOUR_LENGTH, angle);

    hour_point_trace[0] = coordinate_relocate(5, angle);
    hour_point_trace[1] = coordinate_relocate(31, angle);

    lv_line_set_points(hour_body, hour_point, 2);
    lv_line_set_points(hour_body_trace, hour_point_trace, 2);
  }

  if (second.IsUpdated()) {
    auto const angle = second * 6;

    second_point[0] = coordinate_relocate(-20, angle);
    second_point[1] = coordinate_relocate(SECOND_LENGTH, angle);

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
    lv_label_set_text(notificationIcon, icon);
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
