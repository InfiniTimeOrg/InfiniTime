#include "SettingSetDate.h"
#include <lvgl/lvgl.h>
#include <hal/nrf_rtc.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

#define POS_X_DAY   -72
#define POS_X_MONTH   0
#define POS_X_YEAR   72
#define POS_Y_PLUS  -50
#define POS_Y_TEXT   -6
#define POS_Y_MINUS  40

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t * obj, lv_event_t event) {
    SettingSetDate* screen = static_cast<SettingSetDate *>(obj->user_data);
    screen->HandleButtonPress(obj, event);
  }
}

SettingSetDate::SettingSetDate(
  Pinetime::Applications::DisplayApp *app, Pinetime::Controllers::DateTime &dateTimeController) :
  Screen(app),
  dateTimeController {dateTimeController}
{
  lv_obj_t * title = lv_label_create(lv_scr_act(), NULL);  
  lv_label_set_text_static(title, "Set current date");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t * icon = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  
  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  dayValue = static_cast<int>(dateTimeController.Day());
  lblDay = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text_fmt(lblDay, "%d", dayValue);
  lv_label_set_align(lblDay, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblDay, lv_scr_act(), LV_ALIGN_CENTER, POS_X_DAY, POS_Y_TEXT);
  lv_obj_set_auto_realign(lblDay, true);

  monthValue = static_cast<int>(dateTimeController.Month());
  lblMonth = lv_label_create(lv_scr_act(), NULL);
  UpdateMonthLabel();
  lv_label_set_align(lblMonth, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblMonth, lv_scr_act(), LV_ALIGN_CENTER, POS_X_MONTH, POS_Y_TEXT);
  lv_obj_set_auto_realign(lblMonth, true);

  yearValue = static_cast<int>(dateTimeController.Year());
  if (yearValue < 2021)
    yearValue = 2021;
  lblYear = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text_fmt(lblYear, "%d", yearValue);
  lv_label_set_align(lblYear, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(lblYear, lv_scr_act(), LV_ALIGN_CENTER, POS_X_YEAR, POS_Y_TEXT);
  lv_obj_set_auto_realign(lblYear, true);

  btnDayPlus = lv_btn_create(lv_scr_act(), NULL);
  btnDayPlus->user_data = this;
  lv_obj_set_size(btnDayPlus, 50, 40);
  lv_obj_align(btnDayPlus, lv_scr_act(), LV_ALIGN_CENTER, POS_X_DAY, POS_Y_PLUS);
  lv_obj_set_style_local_value_str(btnDayPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
  lv_obj_set_event_cb(btnDayPlus, event_handler);

  btnDayMinus = lv_btn_create(lv_scr_act(), NULL);
  btnDayMinus->user_data = this;
  lv_obj_set_size(btnDayMinus, 50, 40);
  lv_obj_align(btnDayMinus, lv_scr_act(), LV_ALIGN_CENTER, POS_X_DAY, POS_Y_MINUS);
  lv_obj_set_style_local_value_str(btnDayMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
  lv_obj_set_event_cb(btnDayMinus, event_handler);

  btnMonthPlus = lv_btn_create(lv_scr_act(), NULL);
  btnMonthPlus->user_data = this;
  lv_obj_set_size(btnMonthPlus, 50, 40);
  lv_obj_align(btnMonthPlus, lv_scr_act(), LV_ALIGN_CENTER, POS_X_MONTH, POS_Y_PLUS);
  lv_obj_set_style_local_value_str(btnMonthPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
  lv_obj_set_event_cb(btnMonthPlus, event_handler);

  btnMonthMinus = lv_btn_create(lv_scr_act(), NULL);
  btnMonthMinus->user_data = this;
  lv_obj_set_size(btnMonthMinus, 50, 40);
  lv_obj_align(btnMonthMinus, lv_scr_act(), LV_ALIGN_CENTER, POS_X_MONTH, POS_Y_MINUS);
  lv_obj_set_style_local_value_str(btnMonthMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
  lv_obj_set_event_cb(btnMonthMinus, event_handler);

  btnYearPlus = lv_btn_create(lv_scr_act(), NULL);
  btnYearPlus->user_data = this;
  lv_obj_set_size(btnYearPlus, 50, 40);
  lv_obj_align(btnYearPlus, lv_scr_act(), LV_ALIGN_CENTER, POS_X_YEAR, POS_Y_PLUS);
  lv_obj_set_style_local_value_str(btnYearPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");
  lv_obj_set_event_cb(btnYearPlus, event_handler);

  btnYearMinus = lv_btn_create(lv_scr_act(), NULL);
  btnYearMinus->user_data = this;
  lv_obj_set_size(btnYearMinus, 50, 40);
  lv_obj_align(btnYearMinus, lv_scr_act(), LV_ALIGN_CENTER, POS_X_YEAR, POS_Y_MINUS);
  lv_obj_set_style_local_value_str(btnYearMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");
  lv_obj_set_event_cb(btnYearMinus, event_handler);

  btnSetTime = lv_btn_create(lv_scr_act(), NULL);
  btnSetTime->user_data = this;
  lv_obj_set_size(btnSetTime, 120, 48);
  lv_obj_align(btnSetTime, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_local_value_str(btnSetTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Set");
  lv_obj_set_event_cb(btnSetTime, event_handler);
}

SettingSetDate::~SettingSetDate() {
  lv_obj_clean(lv_scr_act());
}

void SettingSetDate::HandleButtonPress(lv_obj_t *object, lv_event_t event) {

  if (event != LV_EVENT_CLICKED)
    return;

  if (object == btnDayPlus) {
    dayValue++;
    if (dayValue > MaximumDayOfMonth())
      dayValue = 1;
    lv_label_set_text_fmt(lblDay, "%d", dayValue);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
  }
  else if (object == btnDayMinus) {
    dayValue--;
    if (dayValue < 1)
      dayValue = MaximumDayOfMonth();
    lv_label_set_text_fmt(lblDay, "%d", dayValue);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
  }
  else if (object == btnMonthPlus) {
    monthValue++;
    if (monthValue > 12)
      monthValue = 1;
    UpdateMonthLabel();
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
    CheckDay();
  }
  else if (object == btnMonthMinus) {
    monthValue--;
    if (monthValue < 1)
      monthValue = 12;
    UpdateMonthLabel();
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
    CheckDay();
  }
  else if (object == btnYearPlus) {
    yearValue++;
    lv_label_set_text_fmt(lblYear, "%d", yearValue);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
    CheckDay();
  }
  else if (object == btnYearMinus) {
    yearValue--;
    lv_label_set_text_fmt(lblYear, "%d", yearValue);
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_RELEASED);
    CheckDay();
  }
  else if (object == btnSetTime) {
    NRF_LOG_INFO("Setting date (manually) to %04d-%02d-%02d", yearValue, monthValue, dayValue);
    dateTimeController.SetTime(static_cast<uint16_t>(yearValue),
                               static_cast<uint8_t>(monthValue),
                               static_cast<uint8_t>(dayValue),
                               0,
                               dateTimeController.Hours(),
                               dateTimeController.Minutes(),
                               dateTimeController.Seconds(),
                               nrf_rtc_counter_get(portNRF_RTC_REG));
    lv_btn_set_state(btnSetTime, LV_BTN_STATE_DISABLED);
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
    lv_obj_align(lblDay, lv_scr_act(), LV_ALIGN_CENTER, POS_X_DAY, POS_Y_TEXT);
  }
}

void SettingSetDate::UpdateMonthLabel() {
  lv_label_set_text_static(lblMonth,
                           Pinetime::Controllers::DateTime::MonthShortToStringLow(static_cast<Pinetime::Controllers::DateTime::Months>(monthValue)));
}

