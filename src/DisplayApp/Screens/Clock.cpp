#include <cstdio>
#include <libs/date/includes/date/date.h>
#include <Components/DateTime/DateTimeController.h>
#include <Version.h>
#include <libs/lvgl/lvgl.h>
#include "Clock.h"
#include "../DisplayApp.h"
#include "BatteryIcon.h"
#include "BleIcon.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;


static void event_handler(lv_obj_t * obj, lv_event_t event) {
  Clock* screen = static_cast<Clock *>(obj->user_data);
  screen->OnObjectEvent(obj, event);
}

Clock::Clock(DisplayApp* app,
        Controllers::DateTime& dateTimeController,
        Controllers::Battery& batteryController,
        Controllers::Ble& bleController) : Screen(app), currentDateTime{{}}, version {{}},
                                           dateTimeController{dateTimeController}, batteryController{batteryController}, bleController{bleController} {
  displayedChar[0] = 0;
  displayedChar[1] = 0;
  displayedChar[2] = 0;
  displayedChar[3] = 0;
  displayedChar[4] = 0;

  batteryIcon = lv_img_create(lv_scr_act(), NULL);
  lv_img_set_src(batteryIcon, BatteryIcon::GetUnknownIcon());
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  bleIcon = lv_img_create(lv_scr_act(), NULL);
  lv_img_set_src(bleIcon, BleIcon::GetIcon(false));
  lv_obj_align(bleIcon, batteryIcon, LV_ALIGN_OUT_LEFT_MID, 0, 0);

//  label_battery = lv_label_create(lv_scr_act(), NULL);
//  lv_obj_align(label_battery, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -80, 0);

//  labelStyle = const_cast<lv_style_t *>(lv_label_get_style(label_battery, LV_LABEL_STYLE_MAIN));
//  labelStyle->text.font = &jetbrains_mono_bold_20;
//
//  lv_style_copy(&labelBigStyle, labelStyle);
//  labelBigStyle.text.font = &jetbrains_mono_extrabold_compressed;
//
//  lv_label_set_style(label_battery, LV_LABEL_STYLE_MAIN, labelStyle);

//  label_ble = lv_label_create(lv_scr_act(), NULL);


//  lv_label_set_style(label_ble, LV_LABEL_STYLE_MAIN, labelStyle);
//  lv_obj_align(label_ble, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, 0);

  label_date = lv_label_create(lv_scr_act(), NULL);

  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 60);

  labelStyle = const_cast<lv_style_t *>(lv_label_get_style(label_date, LV_LABEL_STYLE_MAIN));
  labelStyle->text.font = &jetbrains_mono_bold_20;

  lv_style_copy(&labelBigStyle, labelStyle);
  labelBigStyle.text.font = &jetbrains_mono_extrabold_compressed;

  lv_label_set_style(label_date, LV_LABEL_STYLE_MAIN, labelStyle);

  label_time = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_style(label_time, LV_LABEL_STYLE_MAIN, &labelBigStyle);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, 0);

  backgroundLabel = lv_label_create(lv_scr_act(), NULL);
  backgroundLabel->user_data = this;
  lv_label_set_style(backgroundLabel, LV_LABEL_STYLE_MAIN, labelStyle);
  lv_obj_set_click(backgroundLabel, true);
  lv_obj_set_event_cb(backgroundLabel, event_handler);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
}

Clock::~Clock() {
  lv_obj_clean(lv_scr_act());
}

bool Clock::Refresh() {
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    lv_img_set_src(batteryIcon, BatteryIcon::GetIcon(batteryController.IsCharging() || batteryController.IsPowerPresent(), batteryPercent));
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    if(bleState.Get() == true) {
      lv_img_set_src(bleIcon, BleIcon::GetIcon(true));
    } else {
      lv_img_set_src(bleIcon, BleIcon::GetIcon(false));
    }
  }

  currentDateTime = dateTimeController.CurrentDateTime();

  if(currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime-dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto year = (int)yearMonthDay.year();
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>((unsigned)yearMonthDay.month());
    auto day = (unsigned)yearMonthDay.day();
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    auto hour = time.hours().count();
    auto minute = time.minutes().count();
    auto second = time.seconds().count();

    char minutesChar[3];
    sprintf(minutesChar, "%02d", minute);

    char hoursChar[3];
    sprintf(hoursChar, "%02d", hour);

    char timeStr[6];
    sprintf(timeStr, "%c%c:%c%c", hoursChar[0],hoursChar[1],minutesChar[0], minutesChar[1]);

    if(hoursChar[0] != displayedChar[0] || hoursChar[1] != displayedChar[1] || minutesChar[0] != displayedChar[2] || minutesChar[1] != displayedChar[3]) {
      displayedChar[0] = hoursChar[0];
      displayedChar[1] = hoursChar[1];
      displayedChar[2] = minutesChar[0];
      displayedChar[3] = minutesChar[1];

      lv_label_set_text(label_time, timeStr);
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      char dateStr[22];
      sprintf(dateStr, "%s %d %s %d", DayOfWeekToString(dayOfWeek), day, MonthToString(month), year);
      lv_label_set_text(label_date, dateStr);


      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  return running;
}

const char *Clock::MonthToString(Pinetime::Controllers::DateTime::Months month) {
  return Clock::MonthsString[static_cast<uint8_t>(month)];
}

const char *Clock::DayOfWeekToString(Pinetime::Controllers::DateTime::Days dayOfWeek) {
  return Clock::DaysString[static_cast<uint8_t>(dayOfWeek)];
}

char const *Clock::DaysString[] = {
        "",
        "MONDAY",
        "TUESDAY",
        "WEDNESDAY",
        "THURSDAY",
        "FRIDAY",
        "SATURDAY",
        "SUNDAY"
};

char const *Clock::MonthsString[] = {
        "",
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC"
};

void Clock::OnObjectEvent(lv_obj_t *obj, lv_event_t event) {
  if(obj == backgroundLabel) {
    if (event == LV_EVENT_CLICKED) {

      running = false;
    }
  }
}

bool Clock::OnButtonPushed() {
  running = false;
  return false;
}


