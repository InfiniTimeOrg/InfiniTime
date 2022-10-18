#include "displayapp/screens/settings/SettingChimes.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void onFrequencyChange(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingChimes*>(obj->user_data);
    screen->UpdateFrequency(obj, event);
  }
}

constexpr std::array<SettingChimes::Option, 4> SettingChimes::frequencyOptions;

SettingChimes::SettingChimes(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Chimes");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  uint8_t selectedFrequency = settingsController.GetChimesFrequency();
  for (unsigned int i = 0; i < frequencyOptions.size(); i++) {
    frequencyCheckBoxes[i] = lv_checkbox_create(container1, nullptr);
    lv_checkbox_set_text(frequencyCheckBoxes[i], frequencyOptions[i].label);
    if (selectedFrequency == frequencyOptions[i].value) {
      lv_checkbox_set_checked(frequencyCheckBoxes[i], true);
    }
    frequencyCheckBoxes[i]->user_data = this;
    lv_obj_set_event_cb(frequencyCheckBoxes[i], onFrequencyChange);
    SetRadioButtonStyle(frequencyCheckBoxes[i]);
  }
}

SettingChimes::~SettingChimes() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingChimes::UpdateFrequency(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    for (uint8_t i = 0; i < frequencyOptions.size(); i++) {
      if (object == frequencyCheckBoxes[i]) {
        lv_checkbox_set_checked(frequencyCheckBoxes[i], true);
        settingsController.SetChimesFrequency(frequencyOptions[i].value);
      } else {
        lv_checkbox_set_checked(frequencyCheckBoxes[i], false);
      }
    }
  }
}
