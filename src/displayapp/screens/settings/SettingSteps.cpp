#include "displayapp/screens/settings/SettingSteps.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingSteps* screen = static_cast<SettingSteps*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingSteps::SettingSteps(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_pos(container1, 30, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 50);
  lv_obj_set_height(container1, LV_VER_RES - 60);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Daily steps goal");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);

  lv_label_set_text_static(icon, Symbols::shoe);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(stepValue, "%lu", settingsController.GetStepsGoal());
  lv_label_set_align(stepValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -20);

  static constexpr uint8_t btnWidth = 115;
  static constexpr uint8_t btnHeight = 80;

  btnPlus = lv_btn_create(lv_scr_act(), nullptr);
  btnPlus->user_data = this;
  lv_obj_set_size(btnPlus, btnWidth, btnHeight);
  lv_obj_align(btnPlus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_local_bg_color(btnPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblPlus = lv_label_create(btnPlus, nullptr);
  lv_obj_set_style_local_text_font(lblPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblPlus, "+");
  lv_obj_set_event_cb(btnPlus, event_handler);

  btnMinus = lv_btn_create(lv_scr_act(), nullptr);
  btnMinus->user_data = this;
  lv_obj_set_size(btnMinus, btnWidth, btnHeight);
  lv_obj_set_event_cb(btnMinus, event_handler);
  lv_obj_align(btnMinus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblMinus = lv_label_create(btnMinus, nullptr);
  lv_obj_set_style_local_text_font(lblMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblMinus, "-");
}

SettingSteps::~SettingSteps() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingSteps::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  uint32_t value = settingsController.GetStepsGoal();

  int valueChange = 0;
  if (event == LV_EVENT_SHORT_CLICKED) {
    valueChange = 500;
  } else if (event == LV_EVENT_LONG_PRESSED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
    valueChange = 1000;
  } else {
    return;
  }

  if (object == btnPlus) {
    value += valueChange;
  } else if (object == btnMinus) {
    value -= valueChange;
  }

  if (value >= 1000 && value <= 500000) {
    settingsController.SetStepsGoal(value);
    lv_label_set_text_fmt(stepValue, "%lu", settingsController.GetStepsGoal());
    lv_obj_realign(stepValue);
  }
}
