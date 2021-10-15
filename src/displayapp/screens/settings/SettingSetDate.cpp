#include "SettingSetDate.h"
#include <lvgl/lvgl.h>
#include <hal/nrf_rtc.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t POS_X_DAY = -72;
  constexpr int16_t POS_X_MONTH = 0;
  constexpr int16_t POS_X_YEAR = 72;
  constexpr int16_t POS_Y_PLUS = -50;
  constexpr int16_t POS_Y_TEXT = -6;
  constexpr int16_t POS_Y_MINUS = 40;

  void event_handler(lv_event_t *event) {
    auto* screen = static_cast<SettingSetDate *>(lv_event_get_user_data(event));
    screen->HandleButtonPress(lv_event_get_target(event), event);
  }
}

SettingSetDate::SettingSetDate(Pinetime::Applications::DisplayApp *app, Pinetime::Controllers::DateTime &dateTimeController) :
  Screen(app),
  dateTimeController {dateTimeController} {
  lv_obj_t * title = lv_label_create(lv_scr_act());
  lv_label_set_text_static(title, "Set current date");
  lv_obj_set_style_text_align(title,  LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t * icon = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(icon, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  
  lv_label_set_text_static(icon, Symbols::clock);
  lv_obj_set_style_text_align(icon,  LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  dayValue = static_cast<int>(dateTimeController.Day());
  lblDay = lv_label_create(lv_scr_act());
  lv_label_set_text_fmt(lblDay, "%d", dayValue);
  lv_obj_set_style_text_align(lblDay,  LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(lblDay, LV_ALIGN_CENTER, POS_X_DAY, POS_Y_TEXT);

  monthValue = static_cast<int>(dateTimeController.Month());
  lblMonth = lv_label_create(lv_scr_act());
  UpdateMonthLabel();
  lv_obj_set_style_text_align(lblMonth,  LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(lblMonth, LV_ALIGN_CENTER, POS_X_MONTH, POS_Y_TEXT);


  yearValue = static_cast<int>(dateTimeController.Year());
  if (yearValue < 2021)
    yearValue = 2021;
  lblYear = lv_label_create(lv_scr_act());
  lv_label_set_text_fmt(lblYear, "%d", yearValue);
  lv_obj_set_style_text_align(lblYear,  LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(lblYear, LV_ALIGN_CENTER, POS_X_YEAR, POS_Y_TEXT);

  btnDayPlus = lv_btn_create(lv_scr_act());
  btnDayPlus->user_data = this;
  lv_obj_set_size(btnDayPlus, 50, 40);
  lv_obj_align(btnDayPlus, LV_ALIGN_CENTER, POS_X_DAY, POS_Y_PLUS);
  lblDayPlus = lv_label_create(btnDayPlus);
  lv_label_set_text(lblDayPlus, "+");
  lv_obj_center(lblDayPlus);
  lv_obj_add_event_cb(btnDayPlus, event_handler, LV_EVENT_ALL, btnDayPlus->user_data);

  btnDayMinus = lv_btn_create(lv_scr_act());
  btnDayMinus->user_data = this;
  lv_obj_set_size(btnDayMinus, 50, 40);
  lv_obj_align(btnDayMinus, LV_ALIGN_CENTER, POS_X_DAY, POS_Y_MINUS);
  lblDayMinus = lv_label_create(btnDayMinus);
  lv_label_set_text(lblDayMinus, "-");
  lv_obj_center(lblDayMinus);
  lv_obj_add_event_cb(btnDayMinus, event_handler, LV_EVENT_ALL, btnDayMinus->user_data);

  btnMonthPlus = lv_btn_create(lv_scr_act());
  btnMonthPlus->user_data = this;
  lv_obj_set_size(btnMonthPlus, 50, 40);
  lv_obj_align(btnMonthPlus, LV_ALIGN_CENTER, POS_X_MONTH, POS_Y_PLUS);
  lblMonthPlus = lv_label_create(btnMonthPlus);
  lv_label_set_text(lblMonthPlus, "+");
  lv_obj_center(lblMonthPlus);
  lv_obj_add_event_cb(btnMonthPlus, event_handler, LV_EVENT_ALL, btnMonthPlus->user_data);

  btnMonthMinus = lv_btn_create(lv_scr_act());
  btnMonthMinus->user_data = this;
  lv_obj_set_size(btnMonthMinus, 50, 40);
  lv_obj_align(btnMonthMinus, LV_ALIGN_CENTER, POS_X_MONTH, POS_Y_MINUS);
  lblMonthMinus = lv_label_create(btnMonthMinus);
  lv_label_set_text(lblMonthMinus, "-");
  lv_obj_center(lblMonthMinus);
  lv_obj_add_event_cb(btnMonthMinus, event_handler, LV_EVENT_ALL, btnMonthMinus->user_data);

  btnYearPlus = lv_btn_create(lv_scr_act());
  btnYearPlus->user_data = this;
  lv_obj_set_size(btnYearPlus, 50, 40);
  lv_obj_align(btnYearPlus, LV_ALIGN_CENTER, POS_X_YEAR, POS_Y_PLUS);
  lblYearPlus = lv_label_create(btnYearPlus);
  lv_label_set_text(lblYearPlus, "+");
  lv_obj_center(lblYearPlus);
  lv_obj_add_event_cb(btnYearPlus, event_handler, LV_EVENT_ALL, btnYearPlus->user_data);

  btnYearMinus = lv_btn_create(lv_scr_act());
  btnYearMinus->user_data = this;
  lv_obj_set_size(btnYearMinus, 50, 40);
  lv_obj_align(btnYearMinus, LV_ALIGN_CENTER, POS_X_YEAR, POS_Y_MINUS);
  lblYearMinus = lv_label_create(btnYearMinus);
  lv_label_set_text(lblYearMinus, "-");
  lv_obj_center(lblYearMinus);
  lv_obj_add_event_cb(btnYearMinus, event_handler, LV_EVENT_ALL, btnYearMinus->user_data);

  btnSetTime = lv_btn_create(lv_scr_act());
  btnSetTime->user_data = this;
  lv_obj_set_size(btnSetTime, 120, 48);
  lv_obj_align(btnSetTime, LV_ALIGN_BOTTOM_MID, 0, 0);
  lblSetTime = lv_label_create(btnSetTime);
  lv_label_set_text(lblSetTime, "Set");
  lv_obj_center(lblSetTime);
  lv_obj_add_event_cb(btnSetTime, event_handler, LV_EVENT_ALL, btnSetTime->user_data);
}

SettingSetDate::~SettingSetDate() {
  lv_obj_clean(lv_scr_act());
}

void SettingSetDate::HandleButtonPress(lv_obj_t *object, lv_event_t* event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    return;

  if (object == btnDayPlus) {
    dayValue++;
    if (dayValue > MaximumDayOfMonth())
      dayValue = 1;
    lv_label_set_text_fmt(lblDay, "%d", dayValue);
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
  } else if (object == btnDayMinus) {
    dayValue--;
    if (dayValue < 1)
      dayValue = MaximumDayOfMonth();
    lv_label_set_text_fmt(lblDay, "%d", dayValue);
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
  } else if (object == btnMonthPlus) {
    monthValue++;
    if (monthValue > 12)
      monthValue = 1;
    UpdateMonthLabel();
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
    CheckDay();
  } else if (object == btnMonthMinus) {
    monthValue--;
    if (monthValue < 1)
      monthValue = 12;
    UpdateMonthLabel();
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
    CheckDay();
  } else if (object == btnYearPlus) {
    yearValue++;
    lv_label_set_text_fmt(lblYear, "%d", yearValue);
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
    CheckDay();
  } else if (object == btnYearMinus) {
    yearValue--;
    lv_label_set_text_fmt(lblYear, "%d", yearValue);
    lv_obj_clear_state(btnSetTime, LV_STATE_DISABLED);
    CheckDay();
  } else if (object == btnSetTime) {
    NRF_LOG_INFO("Setting date (manually) to %04d-%02d-%02d", yearValue, monthValue, dayValue);
    dateTimeController.SetTime(static_cast<uint16_t>(yearValue),
                               static_cast<uint8_t>(monthValue),
                               static_cast<uint8_t>(dayValue),
                               0,
                               dateTimeController.Hours(),
                               dateTimeController.Minutes(),
                               dateTimeController.Seconds(),
                               nrf_rtc_counter_get(portNRF_RTC_REG));
    lv_obj_add_state(btnSetTime, LV_STATE_DISABLED);
  }
}

int SettingSetDate::MaximumDayOfMonth() const {
  switch (monthValue) {
    case 2:
      if ((((yearValue % 4) == 0) && ((yearValue % 100) != 0)) || ((yearValue % 400) == 0))
        return 29;
      return 28;
    case 4:
    case 6:
    case 9:
    case 11:
      return 30;
    default:
      return 31;
  }
}

void SettingSetDate::CheckDay() {
  int maxDay = MaximumDayOfMonth();
  if (dayValue > maxDay) {
    dayValue = maxDay;
    lv_label_set_text_fmt(lblDay, "%d", dayValue);
    lv_obj_align(lblDay, LV_ALIGN_CENTER, POS_X_DAY, POS_Y_TEXT);
  }
}

void SettingSetDate::UpdateMonthLabel() {
  lv_label_set_text_static(
    lblMonth, Pinetime::Controllers::DateTime::MonthShortToStringLow(static_cast<Pinetime::Controllers::DateTime::Months>(monthValue)));
}
