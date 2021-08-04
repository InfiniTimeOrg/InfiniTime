#include "SettingInterfaceSize.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingInterfaceSize* screen = static_cast<SettingInterfaceSize*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingInterfaceSize::SettingInterfaceSize(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
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
  lv_label_set_text_static(title, "UI size");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (uint8_t i = 0; i < optionsTotal; i++) {
    cbOption[i] = lv_checkbox_create(container1, nullptr);
    cbOption[i]->user_data = this;
    lv_obj_set_event_cb(cbOption[i], event_handler);
  }

  lv_checkbox_set_text_static(cbOption[0], " Normal");
  lv_checkbox_set_text_static(cbOption[1], " Large");

  if (settingsController.GetInterfaceSize() == Controllers::Settings::InterfaceSize::Normal) {
    lv_checkbox_set_checked(cbOption[0], true);
  } else if (settingsController.GetInterfaceSize() == Controllers::Settings::InterfaceSize::Large) {
    lv_checkbox_set_checked(cbOption[1], true);
  }
}

SettingInterfaceSize::~SettingInterfaceSize() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingInterfaceSize::Refresh() {
  return running;
}

void SettingInterfaceSize::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    for (int i = 0; i < optionsTotal; i++) {
      if (object == cbOption[i]) {
        lv_checkbox_set_checked(cbOption[i], true);

        if (i == 0) {
          settingsController.SetInterfaceSize(Controllers::Settings::InterfaceSize::Normal);
        };
        if (i == 1) {
          settingsController.SetInterfaceSize(Controllers::Settings::InterfaceSize::Large);
        };

      } else {
        lv_checkbox_set_checked(cbOption[i], false);
      }
    }
  }
}
