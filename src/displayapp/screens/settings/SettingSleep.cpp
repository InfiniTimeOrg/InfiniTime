#include "displayapp/screens/settings/SettingSleep.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Styles.h"

using namespace Pinetime::Applications::Screens;

constexpr std::array<SettingSleep::Option, 5> SettingSleep::options;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingSleep*>(obj->user_data);
    if (event == LV_EVENT_VALUE_CHANGED) {
      screen->UpdateSelected(obj);
    }
  }
}

SettingSleep::SettingSleep(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {
  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 35);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 20);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Sleep Actions");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::crescentMoon);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (unsigned int i = 0; i < options.size(); i++) {
    cbOption[i] = lv_checkbox_create(container1, nullptr);
    lv_checkbox_set_text(cbOption[i], options[i].name);
    if (settingsController.isSleepOptionOn(static_cast<Controllers::Settings::SleepOption>(i))) {
      lv_checkbox_set_checked(cbOption[i], true);
    }
    cbOption[i]->user_data = this;
    lv_obj_set_event_cb(cbOption[i], event_handler);
  }
}

SettingSleep::~SettingSleep() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingSleep::UpdateSelected(lv_obj_t* object) {
  // Find the index of the checkbox that triggered the event
  for (size_t i = 0; i < options.size(); i++) {
    if (cbOption[i] == object) {
      bool currentState = settingsController.isSleepOptionOn(options[i].sleepOption);
      settingsController.setSleepOption(options[i].sleepOption, !currentState);
      break;
    }
  }

  // Update checkbox according to current sleep options.
  // This is needed because we can have extra logic when setting or unsetting sleep options,
  // for example, when setting AllowNotify, DisableBle is unset and vice versa.
  auto sleepOptions = settingsController.getSleepOptions();
  for (size_t i = 0; i < options.size(); ++i) {
    lv_checkbox_set_checked(cbOption[i], sleepOptions[i]);
  }
}
