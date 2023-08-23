#include "displayapp/screens/settings/SettingSetDate.h"
#include "displayapp/screens/settings/SettingSetDateTimeFormat.h"
#include <lvgl/lvgl.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t BUTTON_WIDTH = 100;
  constexpr int16_t BUTTON_HEIGHT = 60;
  constexpr int16_t BUTTON_SPACING = BUTTON_HEIGHT / 2 + 5;
  constexpr int16_t BUTTON_PADDING = 15;
  constexpr lv_color_t ACTIVE_COLOR = Colors::lightGray;
  constexpr lv_color_t INACTIVE_COLOR = Colors::bgAlt;

  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingSetDateTimeFormat*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->HandleButtonPress(obj);
    }
  }
}

SettingSetDateTimeFormat::SettingSetDateTimeFormat(Pinetime::Controllers::DateTime& dateTimeController,
                                                   Pinetime::Controllers::Settings& settingsController,
                                                   Pinetime::Applications::Screens::SettingSetDateTime& settingSetDateTime)
  : dateTimeController {dateTimeController}, settingsController {settingsController}, settingSetDateTime {settingSetDateTime} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Set format");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);

  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  // TODO set color to match the other screens

  btnTime12hr = lv_btn_create(lv_scr_act(), nullptr);
  btnTime12hr->user_data = this;
  lv_obj_t* label12hr = lv_label_create(btnTime12hr, nullptr);
  lv_label_set_text_static(label12hr, "12 hr");
  lv_obj_set_size(btnTime12hr, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_align(btnTime12hr, nullptr, LV_ALIGN_IN_LEFT_MID, BUTTON_PADDING, BUTTON_SPACING);
  lv_obj_set_event_cb(btnTime12hr, event_handler);

  btnTime24hr = lv_btn_create(lv_scr_act(), nullptr);
  btnTime24hr->user_data = this;
  lv_obj_t* label24hr = lv_label_create(btnTime24hr, nullptr);
  lv_label_set_text_static(label24hr, "24 hr");
  lv_obj_set_size(btnTime24hr, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_align(btnTime24hr, nullptr, LV_ALIGN_IN_LEFT_MID, BUTTON_PADDING, -BUTTON_SPACING);
  lv_obj_set_event_cb(btnTime24hr, event_handler);

  btnDateDmy = lv_btn_create(lv_scr_act(), nullptr);
  btnDateDmy->user_data = this;
  lv_obj_t* labelDmy = lv_label_create(btnDateDmy, nullptr);
  lv_label_set_text_static(labelDmy, "D/M/Y");
  lv_obj_set_size(btnDateDmy, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_align(btnDateDmy, nullptr, LV_ALIGN_IN_RIGHT_MID, -BUTTON_PADDING, BUTTON_SPACING);
  lv_obj_set_event_cb(btnDateDmy, event_handler);

  btnDateMdy = lv_btn_create(lv_scr_act(), nullptr);
  btnDateMdy->user_data = this;
  lv_obj_t* labelMdy = lv_label_create(btnDateMdy, nullptr);
  lv_label_set_text_static(labelMdy, "M/D/Y");
  lv_obj_set_size(btnDateMdy, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_align(btnDateMdy, nullptr, LV_ALIGN_IN_RIGHT_MID, -BUTTON_PADDING, -BUTTON_SPACING);
  lv_obj_set_event_cb(btnDateMdy, event_handler);

  this->SetSelectedColors();
}

SettingSetDateTimeFormat::~SettingSetDateTimeFormat() {
  lv_obj_clean(lv_scr_act());
}

void SettingSetDateTimeFormat::SetSelectedColors() {
  switch (settingsController.GetClockType()) {
    case Pinetime::Controllers::Settings::ClockType::H12:
      lv_obj_set_style_local_bg_color(this->btnTime12hr, LV_PAGE_PART_BG, LV_STATE_DEFAULT, ACTIVE_COLOR);
      lv_obj_set_style_local_bg_color(this->btnTime24hr, LV_PAGE_PART_BG, LV_STATE_DEFAULT, INACTIVE_COLOR);
      break;
    case Pinetime::Controllers::Settings::ClockType::H24:
      lv_obj_set_style_local_bg_color(this->btnTime12hr, LV_PAGE_PART_BG, LV_STATE_DEFAULT, INACTIVE_COLOR);
      lv_obj_set_style_local_bg_color(this->btnTime24hr, LV_PAGE_PART_BG, LV_STATE_DEFAULT, ACTIVE_COLOR);
      break;
    default:
      // this shouldn't happen
      return;
  }

  switch (settingsController.GetDateType()) {
    case Pinetime::Controllers::Settings::DateType::DMY:
      lv_obj_set_style_local_bg_color(this->btnDateDmy, LV_PAGE_PART_BG, LV_STATE_DEFAULT, ACTIVE_COLOR);
      lv_obj_set_style_local_bg_color(this->btnDateMdy, LV_PAGE_PART_BG, LV_STATE_DEFAULT, INACTIVE_COLOR);
      break;
    case Pinetime::Controllers::Settings::DateType::MDY:
      lv_obj_set_style_local_bg_color(this->btnDateDmy, LV_PAGE_PART_BG, LV_STATE_DEFAULT, INACTIVE_COLOR);
      lv_obj_set_style_local_bg_color(this->btnDateMdy, LV_PAGE_PART_BG, LV_STATE_DEFAULT, ACTIVE_COLOR);
      break;
    default:
      // this shoudln't happen
      return;
  }
}

void SettingSetDateTimeFormat::HandleButtonPress(lv_obj_t* buttonPressed) {
  if (buttonPressed == this->btnTime12hr) {
    this->settingsController.SetClockType(Pinetime::Controllers::Settings::ClockType::H12);
  } else if (buttonPressed == this->btnTime24hr) {
    this->settingsController.SetClockType(Pinetime::Controllers::Settings::ClockType::H24);
  } else if (buttonPressed == this->btnDateDmy) {
    this->settingsController.SetDateType(Pinetime::Controllers::Settings::DateType::DMY);
  } else if (buttonPressed == this->btnDateMdy) {
    this->settingsController.SetDateType(Pinetime::Controllers::Settings::DateType::MDY);
  }
  this->SetSelectedColors();
}
