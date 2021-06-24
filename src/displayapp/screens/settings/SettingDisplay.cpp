#include "SettingDisplay.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Messages.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "lv_i18n/lv_i18n.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingDisplay* screen = static_cast<SettingDisplay*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingDisplay::SettingDisplay(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
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
  lv_label_set_text_static(title, _("settings_display_title"));
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::sun);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  optionsTotal = 0;
  cbOption[optionsTotal] = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text_static(cbOption[optionsTotal], _("settings_display_5seconds"));
  cbOption[optionsTotal]->user_data = this;
  lv_obj_set_event_cb(cbOption[optionsTotal], event_handler);
  if (settingsController.GetScreenTimeOut() == 5000) {
    lv_checkbox_set_checked(cbOption[optionsTotal], true);
  }
  optionsTotal++;
  cbOption[optionsTotal] = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text_static(cbOption[optionsTotal], _("settings_display_15seconds"));
  cbOption[optionsTotal]->user_data = this;
  lv_obj_set_event_cb(cbOption[optionsTotal], event_handler);
  if (settingsController.GetScreenTimeOut() == 15000) {
    lv_checkbox_set_checked(cbOption[optionsTotal], true);
  }
  optionsTotal++;
  cbOption[optionsTotal] = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text_static(cbOption[optionsTotal], _("settings_display_20seconds"));
  cbOption[optionsTotal]->user_data = this;
  lv_obj_set_event_cb(cbOption[optionsTotal], event_handler);
  if (settingsController.GetScreenTimeOut() == 20000) {
    lv_checkbox_set_checked(cbOption[optionsTotal], true);
  }
  optionsTotal++;
  cbOption[optionsTotal] = lv_checkbox_create(container1, nullptr);
  lv_checkbox_set_text_static(cbOption[optionsTotal], _("settings_display_30seconds"));
  cbOption[optionsTotal]->user_data = this;
  lv_obj_set_event_cb(cbOption[optionsTotal], event_handler);
  if (settingsController.GetScreenTimeOut() == 30000) {
    lv_checkbox_set_checked(cbOption[optionsTotal], true);
  }
  optionsTotal++;
}

SettingDisplay::~SettingDisplay() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingDisplay::Refresh() {
  return running;
}

void SettingDisplay::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    for (int i = 0; i < optionsTotal; i++) {
      if (object == cbOption[i]) {
        lv_checkbox_set_checked(cbOption[i], true);

        if (i == 0) {
          settingsController.SetScreenTimeOut(5000);
        };
        if (i == 1) {
          settingsController.SetScreenTimeOut(15000);
        };
        if (i == 2) {
          settingsController.SetScreenTimeOut(20000);
        };
        if (i == 3) {
          settingsController.SetScreenTimeOut(30000);
        };

        app->PushMessage(Applications::Display::Messages::UpdateTimeOut);

      } else {
        lv_checkbox_set_checked(cbOption[i], false);
      }
    }
  }
}
