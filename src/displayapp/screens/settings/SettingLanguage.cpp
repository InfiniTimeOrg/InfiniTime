
#include "SettingLanguage.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "lv_i18n/lv_i18n.h"
#include "lvgl/src/lv_core/lv_disp.h"
#include "lvgl/src/lv_core/lv_obj.h"
#include "lvgl/src/lv_misc/lv_area.h"
#include "lvgl/src/lv_widgets/lv_checkbox.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingLanguage* screen = static_cast<SettingLanguage*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingLanguage::SettingLanguage(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
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
  lv_label_set_text_static(title, _("settings_language"));
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  const char* current_locale = lv_i18n_get_current_locale();
  int ix = 0;
  while (lv_i18n_language_pack[ix] != nullptr) {
    lv_i18n_language_pack_t lp = lv_i18n_language_pack[ix];
    lv_obj_t* cb = lv_checkbox_create(container1, nullptr);
    cb->user_data = this;
    lv_obj_set_event_cb(cb, event_handler);
    const char* locale = lp->locale_name;
    lv_checkbox_set_text_static(cb, locale);
    bool checked = strcmp(locale, current_locale) == 0;
    lv_checkbox_set_checked(cb, checked);
    ix++;
  }
}

SettingLanguage::~SettingLanguage() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingLanguage::Refresh() {
  return running;
}

void SettingLanguage::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    const char* selected_locale = lv_checkbox_get_text(object);
    settingsController.SetLocale(selected_locale);
    /*
    lv_i18n_set_locale(selected_locale);
    for (int i = 0; i < optionsTotal; i++) {
      if (object == cbOption[i]) {
        lv_checkbox_set_checked(cbOption[i], true);

        if (i == 0) {
          settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::None);
        };
        if (i == 1) {
          settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::SingleTap);
        };
        if (i == 2) {
          settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap);
        };
        if (i == 3) {
          settingsController.setWakeUpMode(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist);
        };

      } else {
        lv_checkbox_set_checked(cbOption[i], false);
      }
    }
    */
  }
}
