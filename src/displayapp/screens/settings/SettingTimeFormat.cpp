#include "SettingTimeFormat.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_event_t *event) {
    SettingTimeFormat* screen = static_cast<SettingTimeFormat*>(lv_event_get_user_data(event));
    screen->UpdateSelected(lv_event_get_target(event), event);
  }
}

SettingTimeFormat::SettingTimeFormat(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController} {

  lv_obj_t* container1 = lv_obj_create(lv_scr_act());

  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(container1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_row(container1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_column(container1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_obj_set_layout(container1, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

  lv_obj_t* title = lv_label_create(lv_scr_act());
  lv_label_set_text_static(title, "Time format");
  lv_obj_set_style_text_align(title, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(icon, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(icon, Symbols::clock);
  lv_obj_set_style_text_align(icon, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align_to(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  optionsTotal = 0;
  cbOption[optionsTotal] = lv_checkbox_create(container1);
  lv_checkbox_set_text_static(cbOption[optionsTotal], " 12-hour");
  cbOption[optionsTotal]->user_data = this;
  lv_obj_add_event_cb(cbOption[optionsTotal], event_handler, LV_EVENT_ALL, cbOption[optionsTotal]->user_data);
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
    lv_obj_add_state(cbOption[optionsTotal], LV_STATE_CHECKED);
  }

  optionsTotal++;
  cbOption[optionsTotal] = lv_checkbox_create(container1);
  lv_checkbox_set_text_static(cbOption[optionsTotal], " 24-hour");
  cbOption[optionsTotal]->user_data = this;
  lv_obj_add_event_cb(cbOption[optionsTotal], event_handler, LV_EVENT_ALL, cbOption[optionsTotal]->user_data);
  if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
    lv_obj_add_state(cbOption[optionsTotal], LV_STATE_CHECKED);
  }
  optionsTotal++;
}

SettingTimeFormat::~SettingTimeFormat() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingTimeFormat::UpdateSelected(lv_obj_t* object, lv_event_t* event) {
  if (lv_event_get_code(event) == LV_EVENT_VALUE_CHANGED) {
    for (int i = 0; i < optionsTotal; i++) {
      if (object == cbOption[i]) {
        lv_obj_add_state(cbOption[i], LV_STATE_CHECKED);

        if (i == 0) {
          settingsController.SetClockType(Controllers::Settings::ClockType::H12);
        };
        if (i == 1) {
          settingsController.SetClockType(Controllers::Settings::ClockType::H24);
        };

      } else {
        lv_obj_clear_state(cbOption[i], LV_STATE_CHECKED);
      }
    }
  }
}
