#include "displayapp/screens/settings/SettingSetTime.h"
#include <lvgl/lvgl.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t POS_Y_TEXT = -7;

  void SetTimeEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingSetTime*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->SetTime();
    }
  }

  void ValueChangedHandler(void* userData) {
    auto* screen = static_cast<SettingSetTime*>(userData);
    screen->UpdateScreen();
  }
}

SettingSetTime::SettingSetTime(Pinetime::Controllers::DateTime& dateTimeController,
                               Pinetime::Controllers::Settings& settingsController,
                               Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime)
  : dateTimeController {dateTimeController}, settingsController {settingsController}, settingSetDateTime {settingSetDateTime} {

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
  lv_obj_align(staticLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, POS_Y_TEXT);

  hourCounter.Create();
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    hourCounter.EnableTwelveHourMode();
  }
  hourCounter.SetValue(dateTimeController.Hours());
  lv_obj_align(hourCounter.GetObject(), nullptr, LV_ALIGN_CENTER, -75, POS_Y_TEXT);
  hourCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

  minuteCounter.Create();
  minuteCounter.SetValue(dateTimeController.Minutes());
  lv_obj_align(minuteCounter.GetObject(), nullptr, LV_ALIGN_CENTER, 0, POS_Y_TEXT);
  minuteCounter.SetValueChangedEventCallback(this, ValueChangedHandler);

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

  UpdateScreen();
}

SettingSetTime::~SettingSetTime() {
  lv_obj_clean(lv_scr_act());
}

void SettingSetTime::UpdateScreen() {
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    if (hourCounter.GetValue() >= 12) {
      lv_label_set_text_static(lblampm, "PM");
    } else {
      lv_label_set_text_static(lblampm, "AM");
    }
  }
}

void SettingSetTime::SetTime() {
  const int hoursValue = hourCounter.GetValue();
  const int minutesValue = minuteCounter.GetValue();
  NRF_LOG_INFO("Setting time (manually) to %02d:%02d:00", hoursValue, minutesValue);
  dateTimeController.SetTime(dateTimeController.Year(),
                             static_cast<uint8_t>(dateTimeController.Month()),
                             dateTimeController.Day(),
                             static_cast<uint8_t>(hoursValue),
                             static_cast<uint8_t>(minutesValue),
                             0);
  settingSetDateTime.Quit();
}
