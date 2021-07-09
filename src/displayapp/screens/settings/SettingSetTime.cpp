#include "SettingSetTime.h"
#include <lvgl/lvgl.h>
#include <hal/nrf_rtc.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t * obj, lv_event_t event) {
    SettingSetTime* screen = static_cast<SettingSetTime *>(obj->user_data);
    screen->HandleButtonPress(obj, event);
  }
}

SettingSetTime::SettingSetTime(
  Pinetime::Applications::DisplayApp *app, Pinetime::Controllers::DateTime &dateTimeController) :
  Screen(app),
  dateTimeController {dateTimeController}
{

  lv_obj_t * container1 = lv_cont_create(lv_scr_act(), nullptr);

  //lv_obj_set_style_local_bg_color(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_pos(container1, 30, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 50);
  lv_obj_set_height(container1, LV_VER_RES - 60);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t * title = lv_label_create(lv_scr_act(), NULL);  
  lv_label_set_text_static(title, "Set current time");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t * icon = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  
  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  hoursValue = static_cast<int>(dateTimeController.Hours());
  lblHours = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_font(lblHours, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
  lv_label_set_align(lblHours, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblHours, lv_scr_act(), LV_ALIGN_CENTER, -72, -6);

  lv_obj_t * lblColon1 = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_font(lblColon1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblColon1, ":");
  lv_label_set_align(lblColon1, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblColon1, lv_scr_act(), LV_ALIGN_CENTER, -36, -8);

  minutesValue = static_cast<int>(dateTimeController.Minutes());
  lblMinutes = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_font(lblMinutes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(lblMinutes, "%02d", minutesValue);
  lv_label_set_align(lblMinutes, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblMinutes, lv_scr_act(), LV_ALIGN_CENTER, 0, -6);

  lv_obj_t * lblColon2 = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_font(lblColon2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblColon2, ":");
  lv_label_set_align(lblColon2, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblColon2, lv_scr_act(), LV_ALIGN_CENTER, 36, -8);

  lv_obj_t * lblSeconds = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_font(lblSeconds, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblSeconds, "00");
  lv_label_set_align(lblSeconds, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblSeconds, lv_scr_act(), LV_ALIGN_CENTER, 72, -6);

  btnHoursPlus = lv_btn_create(lv_scr_act(), NULL);
  btnHoursPlus->user_data = this;
  lv_obj_set_size(btnHoursPlus, 50, 40);
  lv_obj_align(btnHoursPlus, lv_scr_act(), LV_ALIGN_CENTER, -72, -50);
  lv_obj_set_style_local_value_str(btnHoursPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
  lv_obj_set_event_cb(btnHoursPlus, event_handler);

  btnHoursMinus = lv_btn_create(lv_scr_act(), NULL);
  btnHoursMinus->user_data = this;
  lv_obj_set_size(btnHoursMinus, 50, 40);
  lv_obj_align(btnHoursMinus, lv_scr_act(), LV_ALIGN_CENTER, -72, 40);
  lv_obj_set_style_local_value_str(btnHoursMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
  lv_obj_set_event_cb(btnHoursMinus, event_handler);

  btnMinutesPlus = lv_btn_create(lv_scr_act(), NULL);
  btnMinutesPlus->user_data = this;
  lv_obj_set_size(btnMinutesPlus, 50, 40);
  lv_obj_align(btnMinutesPlus, lv_scr_act(), LV_ALIGN_CENTER, 0, -50);
  lv_obj_set_style_local_value_str(btnMinutesPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
  lv_obj_set_event_cb(btnMinutesPlus, event_handler);

  btnMinutesMinus = lv_btn_create(lv_scr_act(), NULL);
  btnMinutesMinus->user_data = this;
  lv_obj_set_size(btnMinutesMinus, 50, 40);
  lv_obj_align(btnMinutesMinus, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_local_value_str(btnMinutesMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
  lv_obj_set_event_cb(btnMinutesMinus, event_handler);

  btnSetTime = lv_btn_create(lv_scr_act(), NULL);
  btnSetTime->user_data = this;
  lv_obj_set_size(btnSetTime, 70, 40);
  lv_obj_align(btnSetTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 90);
  lv_obj_set_style_local_value_str(btnSetTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Set");
  lv_obj_set_event_cb(btnSetTime, event_handler);
}

SettingSetTime::~SettingSetTime() {
  lv_obj_clean(lv_scr_act());
}

bool SettingSetTime::Refresh() {
  return running;
}


void SettingSetTime::HandleButtonPress(lv_obj_t *object, lv_event_t event) {

  if(object == btnHoursPlus && (event == LV_EVENT_PRESSED)) {
    hoursValue++;
    if (hoursValue > 23)
      hoursValue = 0;
    lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
    lv_obj_align(lblHours, lv_scr_act(), LV_ALIGN_CENTER, -72, -6);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
  }

  if(object == btnHoursMinus && (event == LV_EVENT_PRESSED)) {
    hoursValue--;
    if (hoursValue < 0)
      hoursValue = 23;
    lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
    lv_obj_align(lblHours, lv_scr_act(), LV_ALIGN_CENTER, -72, -6);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
  }

  if(object == btnMinutesPlus && (event == LV_EVENT_PRESSED)) {
    minutesValue++;
    if (minutesValue > 59)
      minutesValue = 0;
    lv_label_set_text_fmt(lblMinutes, "%02d", minutesValue);
    lv_obj_align(lblMinutes, lv_scr_act(), LV_ALIGN_CENTER, 0, -6);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
  }

  if(object == btnMinutesMinus && (event == LV_EVENT_PRESSED)) {
    minutesValue--;
    if (minutesValue < 0)
      minutesValue = 59;
    lv_label_set_text_fmt(lblMinutes, "%02d", minutesValue);
    lv_obj_align(lblMinutes, lv_scr_act(), LV_ALIGN_CENTER, 0, -6);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
  }

  if(object == btnSetTime && (event == LV_EVENT_PRESSED)) {
    NRF_LOG_INFO("Setting time (manually) to %02d:%02d:00", hoursValue, minutesValue);
    dateTimeController.SetTime(dateTimeController.Year(),
                               static_cast<uint8_t>(dateTimeController.Month()),
                               dateTimeController.Day(),
                               static_cast<uint8_t>(dateTimeController.DayOfWeek()),
                               static_cast<uint8_t>(hoursValue),
                               static_cast<uint8_t>(minutesValue),
                               0,
                               nrf_rtc_counter_get(portNRF_RTC_REG));
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_DISABLED);
  }
}
