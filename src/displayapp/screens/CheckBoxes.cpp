#include "CheckBoxes.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    CheckBoxes* screen = static_cast<CheckBoxes*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

CheckBoxes::CheckBoxes(const char* symbol, const char* titleText, Options *options, DisplayApp* app) : Screen(app), options {options} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 0, 40);
  lv_obj_set_width(container1, LV_HOR_RES);
  lv_obj_set_height(container1, LV_VER_RES - 40);
  lv_cont_set_layout(container1, LV_LAYOUT_GRID);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(title, titleText);
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 5);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text(icon, symbol);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (optionsTotal = 0; optionsTotal < 6; optionsTotal++) {
    if (strcmp(options[optionsTotal].title, "") == 0) {
      break;
    }
  }

  lv_style_init(&buttonStyle);
  lv_style_set_bg_color(&buttonStyle, LV_STATE_DEFAULT, lv_color_hex(0x111111));
  lv_style_set_bg_opa(&buttonStyle, LV_STATE_CHECKED, LV_OPA_30);
  lv_style_set_bg_color(&buttonStyle, LV_STATE_CHECKED, LV_COLOR_AQUA);
  if (optionsTotal <= 4) {
    lv_style_set_radius(&buttonStyle, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  }

  for (uint8_t i = 0; i < optionsTotal; i++) {
    buttons[i] = lv_btn_create(container1, nullptr);
    lv_obj_set_style_local_value_str(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, options[i].title);
    lv_obj_add_style(buttons[i], LV_BTN_PART_MAIN, &buttonStyle);
    if (optionsTotal <= 4) {
      lv_obj_set_size(buttons[i], LV_HOR_RES, 47);
    } else {
      lv_obj_set_size(buttons[i], 117, 64);
    }
    if (options[i].state == true) {
      lv_obj_add_state(buttons[i], LV_STATE_CHECKED);
    }
    buttons[i]->user_data = this;
    lv_obj_set_event_cb(buttons[i], event_handler);
  }
}

CheckBoxes::~CheckBoxes() {
  lv_obj_clean(lv_scr_act());
}

bool CheckBoxes::Refresh() {
  return running;
}

void CheckBoxes::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  for (uint8_t i = 0; i < optionsTotal; i++) {
    if (object == buttons[i]) {
      lv_obj_add_state(buttons[i], LV_STATE_CHECKED);
      options[i].state = true;
    } else {
      lv_obj_clear_state(buttons[i], LV_STATE_CHECKED);
      options[i].state = false;
    }
  }
}
