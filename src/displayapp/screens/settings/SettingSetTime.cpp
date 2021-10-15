#include "SettingSetTime.h"
#include <lvgl/lvgl.h>
#include <hal/nrf_rtc.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t POS_X_HOURS = -72;
  constexpr int16_t POS_X_MINUTES = 0;
  constexpr int16_t POS_X_SECONDS = 72;
  constexpr int16_t POS_Y_PLUS = -50;
  constexpr int16_t POS_Y_TEXT = -6;
  constexpr int16_t POS_Y_MINUS = 40;
  constexpr int16_t OFS_Y_COLON = -2;

  void event_handler(lv_event_t* event) {
    auto* screen = static_cast<SettingSetTime *>(lv_event_get_user_data(event));
    screen->HandleButtonPress(lv_event_get_target(event), event);
  }
}

SettingSetTime::SettingSetTime(Pinetime::Applications::DisplayApp *app, Pinetime::Controllers::DateTime &dateTimeController) :
  Screen(app),
  dateTimeController {dateTimeController} {
  lv_obj_t * title = lv_label_create(lv_scr_act());
  lv_label_set_text_static(title, "Set current time");
  lv_obj_center(title);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t * icon = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(icon, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  
  lv_label_set_text_static(icon, Symbols::clock);
  lv_obj_center(icon);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  hoursValue = static_cast<int>(dateTimeController.Hours());
  lblHours = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(lblHours, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
  lv_obj_set_style_text_align(lblHours, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(lblHours, LV_ALIGN_CENTER, POS_X_HOURS, POS_Y_TEXT);

  lv_obj_t * lblColon1 = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(lblColon1, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(lblColon1, ":");
  lv_obj_set_style_text_align(lblColon1, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(lblColon1, LV_ALIGN_CENTER, (POS_X_HOURS + POS_X_MINUTES) / 2, POS_Y_TEXT + OFS_Y_COLON);

  minutesValue = static_cast<int>(dateTimeController.Minutes());
  lblMinutes = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(lblMinutes, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(lblMinutes, "%02d", minutesValue);
  lv_obj_set_style_text_align(lblMinutes, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(lblMinutes, LV_ALIGN_CENTER, POS_X_MINUTES, POS_Y_TEXT);

  lv_obj_t * lblColon2 = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(lblColon2, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(lblColon2, ":");
  lv_obj_set_style_text_align(lblColon2, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(lblColon2, LV_ALIGN_CENTER, (POS_X_MINUTES + POS_X_SECONDS) / 2, POS_Y_TEXT + OFS_Y_COLON);

  lv_obj_t * lblSeconds = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(lblSeconds, &jetbrains_mono_42, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(lblSeconds, "00");
  lv_obj_set_style_text_align(lblSeconds,  LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(lblSeconds, LV_ALIGN_CENTER, POS_X_SECONDS, POS_Y_TEXT);

  btnHoursPlus = lv_btn_create(lv_scr_act());
  btnHoursPlus->user_data = this;
  lv_obj_set_size(btnHoursPlus, 50, 40);
  lv_obj_align(btnHoursPlus, LV_ALIGN_CENTER, -72, -50);
  lblHoursPlus = lv_label_create(btnHoursPlus);
  lv_label_set_text(lblHoursPlus, "+");
  lv_obj_center(lblHoursPlus);
  lv_obj_add_event_cb(btnHoursPlus, event_handler, LV_EVENT_ALL, btnHoursPlus->user_data);

  btnHoursMinus = lv_btn_create(lv_scr_act());
  btnHoursMinus->user_data = this;
  lv_obj_set_size(btnHoursMinus, 50, 40);
  lv_obj_align(btnHoursMinus, LV_ALIGN_CENTER, -72, 40);
  lblHoursMinus = lv_label_create(btnHoursMinus);
  lv_label_set_text(lblHoursMinus, "-");
  lv_obj_center(lblHoursMinus);
  lv_obj_add_event_cb(btnHoursMinus, event_handler, LV_EVENT_ALL, btnHoursMinus->user_data);

  btnMinutesPlus = lv_btn_create(lv_scr_act());
  btnMinutesPlus->user_data = this;
  lv_obj_set_size(btnMinutesPlus, 50, 40);
  lv_obj_align(btnMinutesPlus, LV_ALIGN_CENTER, 0, -50);
  lblMinutesPlus = lv_label_create(btnMinutesPlus);
  lv_label_set_text(lblMinutesPlus, "+");
  lv_obj_center(lblMinutesPlus);
  lv_obj_add_event_cb(btnMinutesPlus, event_handler, LV_EVENT_ALL, btnMinutesPlus->user_data);

  btnMinutesMinus = lv_btn_create(lv_scr_act());
  btnMinutesMinus->user_data = this;
  lv_obj_set_size(btnMinutesMinus, 50, 40);
  lv_obj_align(btnMinutesMinus, LV_ALIGN_CENTER, 0, 40);
  lblMinutesMinus = lv_label_create(btnMinutesMinus);
  lv_label_set_text(lblMinutesMinus, "-");
  lv_obj_center(lblMinutesMinus);
  lv_obj_add_event_cb(btnMinutesMinus, event_handler, LV_EVENT_ALL, btnMinutesMinus->user_data);

  btnSetTime = lv_btn_create(lv_scr_act());
  btnSetTime->user_data = this;
  lv_obj_set_size(btnSetTime, 120, 48);
  lv_obj_align(btnSetTime, LV_ALIGN_BOTTOM_MID, 0, 0);
  lblSetTime = lv_label_create(btnSetTime);
  lv_label_set_text(lblSetTime, "Set");
  lv_obj_center(lblSetTime);
  lv_obj_add_event_cb(btnSetTime, event_handler, LV_EVENT_ALL, btnSetTime->user_data);
}

SettingSetTime::~SettingSetTime() {
  lv_obj_clean(lv_scr_act());
}

void SettingSetTime::HandleButtonPress(lv_obj_t *object, lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    return;

  if (object == btnHoursPlus) {
    hoursValue++;
    if (hoursValue > 23)
      hoursValue = 0;
    lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
  } else if (object == btnHoursMinus) {
    hoursValue--;
    if (hoursValue < 0)
      hoursValue = 23;
    lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
  } else if (object == btnMinutesPlus) {
    minutesValue++;
    if (minutesValue > 59)
      minutesValue = 0;
    lv_label_set_text_fmt(lblMinutes, "%02d", minutesValue);
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
  } else if (object == btnMinutesMinus) {
    minutesValue--;
    if (minutesValue < 0)
      minutesValue = 59;
    lv_label_set_text_fmt(lblMinutes, "%02d", minutesValue);
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
  } else if (object == btnSetTime) {
    NRF_LOG_INFO("Setting time (manually) to %02d:%02d:00", hoursValue, minutesValue);
    dateTimeController.SetTime(dateTimeController.Year(),
                               static_cast<uint8_t>(dateTimeController.Month()),
                               dateTimeController.Day(),
                               static_cast<uint8_t>(dateTimeController.DayOfWeek()),
                               static_cast<uint8_t>(hoursValue),
                               static_cast<uint8_t>(minutesValue),
                               0,
                               nrf_rtc_counter_get(portNRF_RTC_REG));
    lv_obj_add_state(btnSetTime, LV_STATE_DISABLED);
  }
}
