#include "displayapp/screens/settings/SettingSetDateTime.h"
#include <lvgl/lvgl.h>
#include <hal/nrf_rtc.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/DotLabel.h"
#include "components/settings/Settings.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t POS_X_DAY = -72;
  constexpr int16_t POS_X_MONTH = 0;
  constexpr int16_t POS_X_YEAR = 72;
  constexpr int16_t POS_Y_TEXT = -6;
  constexpr int16_t POS_Y_TEXTD = -7;

  void SetDateEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingSetDateTime*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->SetDate();
    }
  }

  void SetTimeEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingSetDateTime*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->SetTime();
    }
  }

  void DateValueChangedHandler(void* userData) {
    auto* screen = static_cast<SettingSetDateTime*>(userData);
    screen->UpdateDate();
  }

  void TimeValueChangedHandler(void* userData) {
    auto* screen = static_cast<SettingSetDateTime*>(userData);
    screen->UpdateTime();
  }

  int MaximumDayOfMonth(uint8_t month, uint16_t year) {
    switch (month) {
      case 2: {
        if ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0)) {
          return 29;
        }
        return 28;
      }
      case 4:
      case 6:
      case 9:
      case 11:
        return 30;
      default:
        return 31;
    }
  }
}

bool SettingSetDateTime::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

SettingSetDateTime::SettingSetDateTime(Pinetime::Applications::DisplayApp* app,
                                       Pinetime::Controllers::DateTime& dateTimeController,
                                       Pinetime::Controllers::Settings& settingsController)
  : Screen(app),
    dateTimeController {dateTimeController},
    settingsController {settingsController},
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return CreateScreen1();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen2();
              }},
             Screens::ScreenListModes::UpDown} {
}

std::unique_ptr<Screen> SettingSetDateTime::CreateScreen1() {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Set current date");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);

  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  dayCounter.SetValueChangedEventCallback(this, DateValueChangedHandler);
  dayCounter.Create();
  dayCounter.SetValue(dateTimeController.Day());
  lv_obj_align(dayCounter.GetObject(), nullptr, LV_ALIGN_CENTER, POS_X_DAY, POS_Y_TEXT);

  monthCounter.EnableMonthMode();
  monthCounter.SetValueChangedEventCallback(this, DateValueChangedHandler);
  monthCounter.Create();
  monthCounter.SetValue(static_cast<int>(dateTimeController.Month()));
  lv_obj_align(monthCounter.GetObject(), nullptr, LV_ALIGN_CENTER, POS_X_MONTH, POS_Y_TEXT);

  yearCounter.SetValueChangedEventCallback(this, DateValueChangedHandler);
  yearCounter.Create();
  yearCounter.SetValue(dateTimeController.Year());
  lv_obj_align(yearCounter.GetObject(), nullptr, LV_ALIGN_CENTER, POS_X_YEAR, POS_Y_TEXT);

  btnSetDate = lv_btn_create(lv_scr_act(), nullptr);
  btnSetDate->user_data = this;
  lv_obj_set_size(btnSetDate, 120, 48);
  lv_obj_align(btnSetDate, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_local_bg_color(btnSetDate, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x38, 0x38, 0x38));
  lblSetDate = lv_label_create(btnSetDate, nullptr);
  lv_label_set_text_static(lblSetDate, "Set");
  lv_obj_set_event_cb(btnSetDate, SetDateEventHandler);
  lv_btn_set_state(btnSetDate, LV_BTN_STATE_DISABLED);
  lv_obj_set_state(lblSetDate, LV_STATE_DISABLED);

  return std::make_unique<Screens::DotLabel>(0, 2, app, title);
}

std::unique_ptr<Screen> SettingSetDateTime::CreateScreen2() {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Set current time");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  lv_obj_t* staticLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(staticLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(staticLabel, "00:00:00");
  lv_obj_align(staticLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, POS_Y_TEXTD);

  hourCounter.Create();
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    hourCounter.EnableTwelveHourMode();
  }

  hourCounter.SetValue(dateTimeController.Hours());
  lv_obj_align(hourCounter.GetObject(), nullptr, LV_ALIGN_CENTER, -75, POS_Y_TEXTD);
  hourCounter.SetValueChangedEventCallback(this, TimeValueChangedHandler);

  minuteCounter.Create();
  minuteCounter.SetValue(dateTimeController.Minutes());
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_CENTER, 0, POS_Y_TEXTD);
  minuteCounter.SetValueChangedEventCallback(this, TimeValueChangedHandler);

  lblampm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_text_static(lblampm, "  ");
  lv_obj_align(lblampm, lv_scr_act(), LV_ALIGN_CENTER, 75, -50);

  btnSetTime = lv_btn_create(lv_scr_act(), nullptr);
  btnSetTime->user_data = this;
  lv_obj_set_size(btnSetTime, 120, 50);
  lv_obj_align(btnSetTime, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  lblSetTime = lv_label_create(btnSetTime, nullptr);
  lv_label_set_text_static(lblSetTime, "Set");
  lv_obj_set_style_local_bg_color(btnSetTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_text_color(lblSetTime, LV_LABEL_PART_MAIN, LV_STATE_DISABLED, LV_COLOR_GRAY);
  lv_obj_set_event_cb(btnSetTime, SetTimeEventHandler);

  UpdateTime();
  lv_obj_set_state(btnSetTime, LV_STATE_DISABLED);
  lv_obj_set_state(lblSetTime, LV_STATE_DISABLED);

  return std::make_unique<Screens::DotLabel>(1, 2, app, title);
}

SettingSetDateTime::~SettingSetDateTime() {
  lv_obj_clean(lv_scr_act());
}

void SettingSetDateTime::SetDate() {
  const uint16_t yearValue = yearCounter.GetValue();
  const uint8_t monthValue = monthCounter.GetValue();
  const uint8_t dayValue = dayCounter.GetValue();
  NRF_LOG_INFO("Setting date (manually) to %04d-%02d-%02d", yearValue, monthValue, dayValue);
  dateTimeController.SetTime(yearValue,
                             monthValue,
                             dayValue,
                             0,
                             dateTimeController.Hours(),
                             dateTimeController.Minutes(),
                             dateTimeController.Seconds(),
                             nrf_rtc_counter_get(portNRF_RTC_REG));
  lv_btn_set_state(btnSetDate, LV_BTN_STATE_DISABLED);
  lv_obj_set_state(lblSetDate, LV_STATE_DISABLED);
  screens.OnTouchEvent(Pinetime::Applications::TouchEvents::SwipeUp);
}

void SettingSetDateTime::SetTime() {
  const int hoursValue = hourCounter.GetValue();
  const int minutesValue = minuteCounter.GetValue();
  NRF_LOG_INFO("Setting time (manually) to %02d:%02d:00", hoursValue, minutesValue);
  dateTimeController.SetTime(dateTimeController.Year(),
                             static_cast<uint8_t>(dateTimeController.Month()),
                             dateTimeController.Day(),
                             static_cast<uint8_t>(dateTimeController.DayOfWeek()),
                             static_cast<uint8_t>(hoursValue),
                             static_cast<uint8_t>(minutesValue),
                             0,
                             nrf_rtc_counter_get(portNRF_RTC_REG));
  lv_obj_set_state(btnSetTime, LV_STATE_DISABLED);
  lv_obj_set_state(lblSetTime, LV_STATE_DISABLED);
  running = false;
}

void SettingSetDateTime::UpdateDate() {
  const int maxDay = MaximumDayOfMonth(monthCounter.GetValue(), yearCounter.GetValue());
  dayCounter.SetMax(maxDay);
  lv_btn_set_state(btnSetDate, LV_BTN_STATE_RELEASED);
  lv_obj_set_state(lblSetDate, LV_STATE_DEFAULT);
}

void SettingSetDateTime::UpdateTime() {
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    if (hourCounter.GetValue() >= 12) {
      lv_label_set_text_static(lblampm, "PM");
    } else {
      lv_label_set_text_static(lblampm, "AM");
    }
  }
  lv_obj_set_state(btnSetTime, LV_STATE_DEFAULT);
  lv_obj_set_state(lblSetTime, LV_STATE_DEFAULT);
}
