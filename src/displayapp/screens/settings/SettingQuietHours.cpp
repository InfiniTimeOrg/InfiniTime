#include "displayapp/screens/settings/SettingQuietHours.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingQuietHours*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }

  void checkbox_event_handler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
      auto* screen = static_cast<SettingQuietHours*>(obj->user_data);
      screen->ToggleEnabled();
    }
  }
}

SettingQuietHours::SettingQuietHours(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Quiet hours");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::moon);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  enabledCheckbox = lv_checkbox_create(lv_scr_act(), nullptr);
  lv_checkbox_set_text(enabledCheckbox, "Enabled");
  lv_checkbox_set_checked(enabledCheckbox, settingsController.GetQuietHoursEnabled());
  enabledCheckbox->user_data = this;
  lv_obj_set_event_cb(enabledCheckbox, checkbox_event_handler);
  lv_obj_align(enabledCheckbox, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, 55);

  static constexpr uint8_t btnWidth = 50;
  static constexpr uint8_t btnHeight = 40;

  // Start hour row
  lv_obj_t* startLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(startLabel, "Start");
  lv_obj_align(startLabel, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 10, -15);

  btnStartMinus = lv_btn_create(lv_scr_act(), nullptr);
  btnStartMinus->user_data = this;
  lv_obj_set_size(btnStartMinus, btnWidth, btnHeight);
  lv_obj_align(btnStartMinus, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 70, -15);
  lv_obj_set_style_local_bg_color(btnStartMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblStartMinus = lv_label_create(btnStartMinus, nullptr);
  lv_label_set_text_static(lblStartMinus, "-");
  lv_obj_set_event_cb(btnStartMinus, event_handler);

  startValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(startValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_text_fmt(startValue, "%02d:00", settingsController.GetQuietHoursStart());
  lv_obj_align(startValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 132, -15);

  btnStartPlus = lv_btn_create(lv_scr_act(), nullptr);
  btnStartPlus->user_data = this;
  lv_obj_set_size(btnStartPlus, btnWidth, btnHeight);
  lv_obj_align(btnStartPlus, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 195, -15);
  lv_obj_set_style_local_bg_color(btnStartPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblStartPlus = lv_label_create(btnStartPlus, nullptr);
  lv_label_set_text_static(lblStartPlus, "+");
  lv_obj_set_event_cb(btnStartPlus, event_handler);

  // End hour row
  lv_obj_t* endLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(endLabel, "End");
  lv_obj_align(endLabel, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 10, 40);

  btnEndMinus = lv_btn_create(lv_scr_act(), nullptr);
  btnEndMinus->user_data = this;
  lv_obj_set_size(btnEndMinus, btnWidth, btnHeight);
  lv_obj_align(btnEndMinus, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 70, 40);
  lv_obj_set_style_local_bg_color(btnEndMinus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblEndMinus = lv_label_create(btnEndMinus, nullptr);
  lv_label_set_text_static(lblEndMinus, "-");
  lv_obj_set_event_cb(btnEndMinus, event_handler);

  endValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(endValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_text_fmt(endValue, "%02d:00", settingsController.GetQuietHoursEnd());
  lv_obj_align(endValue, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 132, 40);

  btnEndPlus = lv_btn_create(lv_scr_act(), nullptr);
  btnEndPlus->user_data = this;
  lv_obj_set_size(btnEndPlus, btnWidth, btnHeight);
  lv_obj_align(btnEndPlus, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 195, 40);
  lv_obj_set_style_local_bg_color(btnEndPlus, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblEndPlus = lv_label_create(btnEndPlus, nullptr);
  lv_label_set_text_static(lblEndPlus, "+");
  lv_obj_set_event_cb(btnEndPlus, event_handler);
}

SettingQuietHours::~SettingQuietHours() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingQuietHours::ToggleEnabled() {
  bool wasEnabled = settingsController.GetQuietHoursEnabled();
  settingsController.SetQuietHoursEnabled(!wasEnabled);
  if (wasEnabled && settingsController.IsInQuietHours()) {
    settingsController.ExitQuietHours();
  }
  lv_checkbox_set_checked(enabledCheckbox, settingsController.GetQuietHoursEnabled());
}

void SettingQuietHours::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event != LV_EVENT_SHORT_CLICKED && event != LV_EVENT_LONG_PRESSED_REPEAT) {
    return;
  }

  if (object == btnStartPlus) {
    uint8_t val = settingsController.GetQuietHoursStart();
    val = (val + 1) % 24;
    settingsController.SetQuietHoursStart(val);
    lv_label_set_text_fmt(startValue, "%02d:00", val);
    lv_obj_realign(startValue);
  } else if (object == btnStartMinus) {
    uint8_t val = settingsController.GetQuietHoursStart();
    val = (val + 23) % 24;
    settingsController.SetQuietHoursStart(val);
    lv_label_set_text_fmt(startValue, "%02d:00", val);
    lv_obj_realign(startValue);
  } else if (object == btnEndPlus) {
    uint8_t val = settingsController.GetQuietHoursEnd();
    val = (val + 1) % 24;
    settingsController.SetQuietHoursEnd(val);
    lv_label_set_text_fmt(endValue, "%02d:00", val);
    lv_obj_realign(endValue);
  } else if (object == btnEndMinus) {
    uint8_t val = settingsController.GetQuietHoursEnd();
    val = (val + 23) % 24;
    settingsController.SetQuietHoursEnd(val);
    lv_label_set_text_fmt(endValue, "%02d:00", val);
    lv_obj_realign(endValue);
  }
}
