#include "SettingWakeUp.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_event_t *event) {
    SettingWakeUp* screen = static_cast<SettingWakeUp*>(lv_event_get_user_data(event));
    screen->UpdateSelected(lv_event_get_target(event), event);
  }
}

SettingWakeUp::SettingWakeUp(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController} {
  ignoringEvents = false;
  lv_obj_t* container1 = lv_obj_create(lv_scr_act());

  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(container1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_gap(container1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);;
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_obj_set_layout(container1, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);


  lv_obj_t* title = lv_label_create(lv_scr_act());
  lv_label_set_text_static(title, "Wake Up");
  lv_obj_set_style_text_align(title, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(icon, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(icon, Symbols::eye);
  lv_obj_set_style_text_align(icon, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  optionsTotal = 0;
  cbOption[optionsTotal] = lv_checkbox_create(container1);
  lv_checkbox_set_text_static(cbOption[optionsTotal], " Single Tap");
  cbOption[optionsTotal]->user_data = this;
  lv_obj_add_event_cb(cbOption[optionsTotal], event_handler, LV_EVENT_ALL, cbOption[optionsTotal]->user_data);
  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::SingleTap)) {
    lv_obj_add_state(cbOption[optionsTotal], LV_STATE_CHECKED);
  }
  optionsTotal++;
  cbOption[optionsTotal] = lv_checkbox_create(container1);
  lv_checkbox_set_text_static(cbOption[optionsTotal], " Double Tap");
  cbOption[optionsTotal]->user_data = this;
  lv_obj_add_event_cb(cbOption[optionsTotal], event_handler, LV_EVENT_ALL, cbOption[optionsTotal]->user_data);
  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
    lv_obj_add_state(cbOption[optionsTotal], LV_STATE_CHECKED);
  }
  optionsTotal++;
  cbOption[optionsTotal] = lv_checkbox_create(container1);
  lv_checkbox_set_text_static(cbOption[optionsTotal], " Raise Wrist");
  cbOption[optionsTotal]->user_data = this;
  lv_obj_add_event_cb(cbOption[optionsTotal], event_handler, LV_EVENT_ALL, cbOption[optionsTotal]->user_data);
  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist)) {
    lv_obj_add_state(cbOption[optionsTotal], LV_STATE_CHECKED);
  }
  optionsTotal++;
}

SettingWakeUp::~SettingWakeUp() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingWakeUp::UpdateSelected(lv_obj_t* object, lv_event_t* event) {
  using WakeUpMode = Pinetime::Controllers::Settings::WakeUpMode;
  if (lv_event_get_code(event) == LV_EVENT_VALUE_CHANGED && !ignoringEvents) {
    ignoringEvents = true;

    // Find the index of the checkbox that triggered the event
    int index = 0;
    for (; index < optionsTotal; ++index) {
      if (cbOption[index] == object) {
        break;
      }
    }

    // Toggle needed wakeup mode
    auto mode = static_cast<WakeUpMode>(index);
    auto currentState = settingsController.isWakeUpModeOn(mode);
    settingsController.setWakeUpMode(mode, !currentState);

    // Update checkbox according to current wakeup modes.
    // This is needed because we can have extra logic when setting or unsetting wakeup modes,
    // for example, when setting SingleTap, DoubleTap is unset and vice versa.
    auto modes = settingsController.getWakeUpModes();
    for (int i = 0; i < optionsTotal; ++i) {
      if (modes[i]){
        lv_obj_add_state(cbOption[i], LV_STATE_CHECKED);
      } else{
        lv_obj_clear_state(cbOption[i], LV_STATE_CHECKED);
      }
    }

    ignoringEvents = false;
  }
}
