#include "displayapp/screens/settings/SettingScreenBase.h"
#include "displayapp/screens/Styles.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void RadioButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* settingScreen = static_cast<SettingScreenBase*>(obj->user_data);
    if (event == LV_EVENT_VALUE_CHANGED) {
      settingScreen->UpdateSelected(obj);
    }
  }
}

SettingScreenBase::SettingScreenBase(const char* title,
                                     const char* icon,
                                     Entry entries[],
                                     size_t nEntries,
                                     lv_obj_t*(*checkboxObjectArray),
                                     bool radioButtonStyle)
  : checkboxObjectArray {checkboxObjectArray}, nEntries {nEntries} {
  lv_obj_t* container = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container, 10, 60);
  lv_obj_set_width(container, LV_HOR_RES - 20);
  lv_obj_set_height(container, LV_VER_RES - 50);
  lv_cont_set_layout(container, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* titleObj = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(titleObj, title);
  lv_label_set_align(titleObj, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(titleObj, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* iconObj = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(iconObj, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(iconObj, icon);
  lv_label_set_align(iconObj, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(iconObj, titleObj, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (unsigned int i = 0; i < nEntries; i++) {
    checkboxObjectArray[i] = lv_checkbox_create(container, nullptr);
    lv_checkbox_set_text(checkboxObjectArray[i], entries[i].title);
    checkboxObjectArray[i]->user_data = this;
    if (radioButtonStyle) {
      SetRadioButtonStyle(checkboxObjectArray[i]);
      lv_obj_set_event_cb(checkboxObjectArray[i], RadioButtonEventHandler);
    }
    if (entries[i].enabled) {
      lv_checkbox_set_checked(checkboxObjectArray[i], true);
    }
  }
}

void SettingScreenBase::UpdateSelected(lv_obj_t* object) {
  for (unsigned int i = 0; i < nEntries; i++) {
    if (object == checkboxObjectArray[i]) {
      lv_checkbox_set_checked(checkboxObjectArray[i], true);
    } else {
      lv_checkbox_set_checked(checkboxObjectArray[i], false);
    }
  }
}
