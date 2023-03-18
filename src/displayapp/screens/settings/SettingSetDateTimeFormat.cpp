#include "displayapp/screens/settings/SettingSetDate.h"
#include "displayapp/screens/settings/SettingSetDateTimeFormat.h"
#include <lvgl/lvgl.h>
#include <nrf_log.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t POS_X_DAY = -72;
  constexpr int16_t POS_X_MONTH = 0;
  constexpr int16_t POS_X_YEAR = 72;
  constexpr int16_t POS_Y_TEXT = -6;

  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingSetDate*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->HandleButtonPress();
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

  btnSetTime = lv_btn_create(lv_scr_act(), nullptr);
  btnSetTime->user_data = this;
  lv_obj_set_size(btnSetTime, 120, 48);
  lv_obj_align(btnSetTime, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_local_bg_color(btnSetTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x38, 0x38, 0x38));
  lblSetTime = lv_label_create(btnSetTime, nullptr);
  lv_label_set_text_static(lblSetTime, "Set");
  lv_obj_set_event_cb(btnSetTime, event_handler);
}

SettingSetDateTimeFormat::~SettingSetDateTimeFormat() {
  lv_obj_clean(lv_scr_act());
}
