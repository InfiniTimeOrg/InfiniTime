#include "displayapp/screens/settings/SettingHeartRate.h"
#include <lvgl/lvgl.h>
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include <array>
#include <algorithm>

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingHeartRate*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

constexpr std::array<Option, 8> SettingHeartRate::options;

SettingHeartRate::SettingHeartRate(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_cont_set_layout(container1, LV_LAYOUT_PRETTY_TOP);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Backg. Interval");
  lv_label_set_text(title, "Backg. Interval");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_label_set_text_static(icon, Symbols::heartBeat);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  bool isActivated = settingsController.IsHeartRateBackgroundMeasurementActivated();
  unsigned int currentInterval = settingsController.GetHeartRateBackgroundMeasurementInterval();

  for (unsigned int i = 0; i < options.size(); i++) {
    cbOption[i] = lv_checkbox_create(container1, nullptr);
    lv_checkbox_set_text(cbOption[i], options[i].name);
    cbOption[i]->user_data = this;
    lv_obj_set_event_cb(cbOption[i], event_handler);
    SetRadioButtonStyle(cbOption[i]);

    if (!isActivated && options[i].intervalInSeconds == -1) {
      lv_checkbox_set_checked(cbOption[i], true);
    } else if (isActivated && options[i].intervalInSeconds == (int) currentInterval) {
      lv_checkbox_set_checked(cbOption[i], true);
    }
  }
}

SettingHeartRate::~SettingHeartRate() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingHeartRate::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    for (unsigned int i = 0; i < options.size(); i++) {
      if (object == cbOption[i]) {
        lv_checkbox_set_checked(cbOption[i], true);

        int optionInterval = options[i].intervalInSeconds;

        if (optionInterval == -1) {
          settingsController.DeactivateHeartRateBackgroundMeasurement();
        } else {
          settingsController.SetHeartRateBackgroundMeasurementInterval((unsigned int) optionInterval);
        }
      } else {
        lv_checkbox_set_checked(cbOption[i], false);
      }
    }
  }
}
