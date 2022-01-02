#include "displayapp/screens/settings/SettingSetTime.h"
#include <lvgl/lvgl.h>
#include <hal/nrf_rtc.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t POS_X_HOURS = -72;
  constexpr int16_t POS_X_MINUTES = 0;
  constexpr int16_t POS_X_SECONDS = 72;
  constexpr int16_t POS_Y_PLUS = -50;
  constexpr int16_t POS_Y_TEXT = -6;
  constexpr int16_t POS_Y_MINUS = 40;
  constexpr int16_t OFS_Y_COLON = -2;

  void event_handler(lv_obj_t * obj, lv_event_t event) {
    auto* screen = static_cast<SettingSetTime *>(obj->user_data);
    screen->HandleButtonPress(obj, event);
  }
}

SettingSetTime::SettingSetTime(
	Pinetime::Applications::DisplayApp *app,
	Pinetime::Controllers::DateTime& dateTimeController,
	Pinetime::Controllers::Settings& settingsController)
  : Screen(app),
  dateTimeController {dateTimeController},
  settingsController {settingsController} {
  lv_obj_t * title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Set current time");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t * icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  
  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  hoursValue = static_cast<int>(dateTimeController.Hours());
  lblHours = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblHours, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
  lv_label_set_align(lblHours, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblHours, lv_scr_act(), LV_ALIGN_CENTER, POS_X_HOURS, POS_Y_TEXT);
  lv_obj_set_auto_realign(lblHours, true);

  lv_obj_t * lblColon1 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblColon1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblColon1, ":");
  lv_label_set_align(lblColon1, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblColon1, lv_scr_act(), LV_ALIGN_CENTER, (POS_X_HOURS + POS_X_MINUTES) / 2, POS_Y_TEXT + OFS_Y_COLON);

  minutesValue = static_cast<int>(dateTimeController.Minutes());
  lblMinutes = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblMinutes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(lblMinutes, "%02d", minutesValue);
  lv_label_set_align(lblMinutes, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblMinutes, lv_scr_act(), LV_ALIGN_CENTER, POS_X_MINUTES, POS_Y_TEXT);
  lv_obj_set_auto_realign(lblMinutes, true);

  lv_obj_t * lblColon2 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblColon2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblColon2, ":");
  lv_label_set_align(lblColon2, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblColon2, lv_scr_act(), LV_ALIGN_CENTER, (POS_X_MINUTES + POS_X_SECONDS) / 2, POS_Y_TEXT + OFS_Y_COLON);

  lv_obj_t * lblSeconds = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblSeconds, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblSeconds, "00");
  lv_label_set_align(lblSeconds, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblSeconds, lv_scr_act(), LV_ALIGN_CENTER, POS_X_SECONDS, POS_Y_TEXT);

  lblampm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_align(lblampm, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblampm, lv_scr_act(), LV_ALIGN_CENTER, POS_X_SECONDS, POS_Y_TEXT - 40);
  lv_label_set_text_fmt(lblampm, "%02c", ampmValue);
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24)
	lv_label_set_text(lblampm, "  ");
  else {
	if (hoursValue > 11)
      lv_label_set_text(lblampm, "PM");
    else
	  lv_label_set_text(lblampm, "AM");
	if (hoursValue > 12)
	  lv_label_set_text_fmt(lblHours, "%02d", hoursValue-12);
	else if (hoursValue == 0)
	  lv_label_set_text_fmt(lblHours, "%02d", hoursValue+1);
  }

  btnHoursPlus = lv_btn_create(lv_scr_act(), nullptr);
  btnHoursPlus->user_data = this;
  lv_obj_set_size(btnHoursPlus, 50, 40);
  lv_obj_align(btnHoursPlus, lv_scr_act(), LV_ALIGN_CENTER, -72, -50);
  lv_obj_set_style_local_value_str(btnHoursPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
  lv_obj_set_event_cb(btnHoursPlus, event_handler);

  btnHoursMinus = lv_btn_create(lv_scr_act(), nullptr);
  btnHoursMinus->user_data = this;
  lv_obj_set_size(btnHoursMinus, 50, 40);
  lv_obj_align(btnHoursMinus, lv_scr_act(), LV_ALIGN_CENTER, -72, 40);
  lv_obj_set_style_local_value_str(btnHoursMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
  lv_obj_set_event_cb(btnHoursMinus, event_handler);

  btnMinutesPlus = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesPlus->user_data = this;
  lv_obj_set_size(btnMinutesPlus, 50, 40);
  lv_obj_align(btnMinutesPlus, lv_scr_act(), LV_ALIGN_CENTER, 0, -50);
  lv_obj_set_style_local_value_str(btnMinutesPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
  lv_obj_set_event_cb(btnMinutesPlus, event_handler);

  btnMinutesMinus = lv_btn_create(lv_scr_act(), nullptr);
  btnMinutesMinus->user_data = this;
  lv_obj_set_size(btnMinutesMinus, 50, 40);
  lv_obj_align(btnMinutesMinus, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_local_value_str(btnMinutesMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
  lv_obj_set_event_cb(btnMinutesMinus, event_handler);

  btnSetTime = lv_btn_create(lv_scr_act(), nullptr);
  btnSetTime->user_data = this;
  lv_obj_set_size(btnSetTime, 120, 48);
  lv_obj_align(btnSetTime, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_local_value_str(btnSetTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Set");
  lv_obj_set_event_cb(btnSetTime, event_handler);
}

SettingSetTime::~SettingSetTime() {
  lv_obj_clean(lv_scr_act());
}

int timeConvert(int time24H) {
	switch (time24H) {
		case 0:
		  return 12;
		  break;
		case 1 ... 12:
		  return time24H;
		  break;
		case 13 ... 23:
		  return time24H - 12;
		  break;
		default:
		  return 99;
		  break;
	}
}

void SettingSetTime::HandleButtonPress(lv_obj_t *object, lv_event_t event) {
  if (event != LV_EVENT_CLICKED)
    return;

  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
	is24H = true;
  }
  else {
	is24H = false;
  }

  if (object == btnHoursPlus) {
    hoursValue++;
    if (hoursValue > 23)
      hoursValue = 0;
	if (!is24H) {
	  if (hoursValue < 12)
	    lv_label_set_text(lblampm, "AM");
	  else
	    lv_label_set_text(lblampm, "PM");
	}
	if (!is24H)
      lv_label_set_text_fmt(lblHours, "%02d", timeConvert(hoursValue));
	else
	  lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
  } else if (object == btnHoursMinus) {
    hoursValue--;
    if (hoursValue < 0)
      hoursValue = 23;
	if (!is24H) {
	  if (hoursValue < 12)
	    lv_label_set_text(lblampm, "AM");
	  else
	    lv_label_set_text(lblampm, "PM");
	}
	if (!is24H)
      lv_label_set_text_fmt(lblHours, "%02d", timeConvert(hoursValue));
	else
	  lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
  } else if (object == btnMinutesPlus) {
    minutesValue++;
    if (minutesValue > 59)
      minutesValue = 0;
    lv_label_set_text_fmt(lblMinutes, "%02d", minutesValue);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
  } else if (object == btnMinutesMinus) {
    minutesValue--;
    if (minutesValue < 0)
      minutesValue = 59;
    lv_label_set_text_fmt(lblMinutes, "%02d", minutesValue);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
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
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_DISABLED);
  }
}
