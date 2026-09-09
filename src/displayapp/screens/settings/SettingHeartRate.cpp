#include "displayapp/screens/settings/SettingHeartRate.h"
#include <lvgl/lvgl.h>
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/localization/Localization.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Applications::Localization;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingHeartRate*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }

  const char* IntervalLabel(std::optional<uint16_t> interval) {
    if (interval == 30) {
      return " 30s";
    }
    if (interval == 60) {
      return "  1m";
    }
    if (interval == 5 * 60) {
      return "  5m";
    }
    if (interval == 10 * 60) {
      return " 10m";
    }
    if (interval == 30 * 60) {
      return " 30m";
    }
    return "";
  }
}

SettingHeartRate::SettingHeartRate(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {
  lv_obj_t* container = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container, 10, 60);
  lv_obj_set_width(container, LV_HOR_RES - 20);
  lv_obj_set_height(container, LV_VER_RES - 50);
  lv_cont_set_layout(container, LV_LAYOUT_PRETTY_TOP);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, Translate(settingsController.GetLanguage(), StringId::BackgroundInterval));
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_label_set_text_static(icon, Symbols::heartBeat);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  std::optional<uint16_t> currentInterval = settingsController.GetHeartRateBackgroundMeasurementInterval();

  for (std::size_t i = 0; i < options.size(); i++) {
    cbOption[i] = lv_checkbox_create(container, nullptr);
    if (options[i].intervalInSeconds == std::nullopt) {
      lv_checkbox_set_text(cbOption[i], Translate(settingsController.GetLanguage(), StringId::Off));
    } else if (options[i].intervalInSeconds == 0) {
      lv_checkbox_set_text(cbOption[i], Translate(settingsController.GetLanguage(), StringId::Continuous));
    } else {
      lv_checkbox_set_text(cbOption[i], IntervalLabel(options[i].intervalInSeconds));
    }
    cbOption[i]->user_data = this;
    lv_obj_set_event_cb(cbOption[i], EventHandler);
    SetRadioButtonStyle(cbOption[i]);

    if (options[i].intervalInSeconds == currentInterval) {
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
    for (std::size_t i = 0; i < options.size(); i++) {
      if (object == cbOption[i]) {
        lv_checkbox_set_checked(cbOption[i], true);
        settingsController.SetHeartRateBackgroundMeasurementInterval(options[i].intervalInSeconds);
      } else {
        lv_checkbox_set_checked(cbOption[i], false);
      }
    }
  }
}
