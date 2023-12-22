#include "displayapp/screens/settings/SettingQuietHour.h"
#include <lvgl/lvgl.h>
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;
using QuietHour = Pinetime::Controllers::Settings::QuietHour;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingQuietHour*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingQuietHour::SettingQuietHour(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Quiet Hour");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 40);
  lv_obj_set_width(container1, LV_HOR_RES - 10);
  lv_obj_set_height(container1, LV_VER_RES - 30);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  char optionName[2][6] = {
    "Start",
    "End",
  };

  // start time and end time options
  for (uint8_t i = 0; i < 2; i++) {
    // check box
    cbOption[i] = lv_checkbox_create(container1, nullptr);
    lv_checkbox_set_text(cbOption[i], optionName[i]);
    cbOption[i]->user_data = this;
    lv_obj_set_event_cb(cbOption[i], event_handler);

    time_container[i] = lv_cont_create(container1, nullptr);
    lv_obj_set_style_local_bg_opa(time_container[i], LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_size(time_container[i], LV_HOR_RES - 60, 50);

    // hour and minute button
    for (uint8_t j = 0; j < 2; j++) {
      btnTime[i][j] = lv_btn_create(time_container[i], nullptr);
      btnTime[i][j]->user_data = this;
      lv_obj_set_event_cb(btnTime[i][j], event_handler);
      lv_obj_set_size(btnTime[i][j], LV_HOR_RES / 3, 50);
      if (j == 0) {
        lv_obj_align(btnTime[i][j], NULL, LV_ALIGN_IN_LEFT_MID, 0, 0);
        colon[i] = lv_label_create(time_container[i], nullptr);
        lv_label_set_text_static(colon[i], ":");
        lv_obj_align(colon[i], NULL, LV_ALIGN_CENTER, 0, 0);
      } else {
        lv_obj_align(btnTime[i][j], NULL, LV_ALIGN_IN_RIGHT_MID, 0, 0);
      }
      txtTime[i][j] = lv_label_create(btnTime[i][j], nullptr);
      lv_label_set_align(txtTime[i][j], LV_LABEL_ALIGN_CENTER);
    }
  }
  UpdateButton();
}

SettingQuietHour::~SettingQuietHour() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingQuietHour::UpdateButton() {
  QuietHour* saved_options = settingsController.GetQuietHour();

  for (uint8_t i = 0; i < 2; i++) {
    lv_checkbox_set_checked(cbOption[i], saved_options[i].auto_toggle);
    if (saved_options[i].auto_toggle) {
      lv_btn_set_state(btnTime[i][0], LV_BTN_STATE_DISABLED);
      lv_btn_set_state(btnTime[i][1], LV_BTN_STATE_DISABLED);
    } else {
      lv_btn_set_state(btnTime[i][0], LV_BTN_STATE_RELEASED);
      lv_btn_set_state(btnTime[i][1], LV_BTN_STATE_RELEASED);
    }
    lv_label_set_text_fmt(txtTime[i][0], "%02d", saved_options[i].time / 2);        // hour
    lv_label_set_text_fmt(txtTime[i][1], "%02d", (saved_options[i].time % 2) * 30); // minute
  }
}

void SettingQuietHour::UpdateSelected(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    for (uint8_t i = 0; i < 2; i++) {
      if (cbOption[i] == obj) {
        if (lv_checkbox_is_inactive(obj)) {
          return;
        }
        QuietHour* saved_options = settingsController.GetQuietHour();
        if (lv_checkbox_is_checked(obj) != saved_options[i].auto_toggle) {
          saved_options[i].auto_toggle = lv_checkbox_is_checked(obj);
          // if start time == stop time, uncheck the other option
          if (saved_options[i].auto_toggle == true && saved_options[i].auto_toggle == true &&
              saved_options[0].time == saved_options[1].time) {
            lv_checkbox_set_checked(cbOption[!i], false);
            saved_options[!i].auto_toggle = false;
          }
          settingsController.SetQuietHour(saved_options);
        }
        UpdateButton();
        return;
      }
    }
  } else if (event == LV_EVENT_CLICKED) {
    for (uint8_t i = 0; i < 2; i++) {
      for (uint8_t j = 0; j < 2; j++) {
        if (btnTime[i][j] == obj) {
          QuietHour* saved_options = settingsController.GetQuietHour();
          saved_options[i].time = (saved_options[i].time + 2 - j) % 48;
          settingsController.SetQuietHour(saved_options);
          UpdateButton();
          return;
        }
      }
    }
  }
}
