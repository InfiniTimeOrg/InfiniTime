#include "displayapp/screens/settings/SettingWidgets.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Styles.h"

using namespace Pinetime::Applications::Screens;

constexpr std::array<SettingWidgets::Option, 3> SettingWidgets::options;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingWidgets*>(obj->user_data);
    if (event == LV_EVENT_VALUE_CHANGED) {
      screen->UpdateSelected(obj);
    }
  }
}

SettingWidgets::SettingWidgets(Pinetime::Controllers::Settings& settingsController) : settingsController {settingsController} {
  lv_obj_t* container = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container, 10, 35);
  lv_obj_set_width(container, LV_HOR_RES - 20);
  lv_obj_set_height(container, LV_VER_RES - 20);
  lv_cont_set_layout(container, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Widgets");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::wrench);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (unsigned int i = 0; i < options.size(); i++) {
    cbOption[i] = lv_checkbox_create(container, nullptr);
    lv_checkbox_set_text(cbOption[i], options[i].name);
    if (settingsController.IsWidgetOn(static_cast<Controllers::Settings::Widget>(i))) {
      lv_checkbox_set_checked(cbOption[i], true);
    }
    cbOption[i]->user_data = this;
    lv_obj_set_event_cb(cbOption[i], event_handler);
  }
}

SettingWidgets::~SettingWidgets() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingWidgets::UpdateSelected(lv_obj_t* object) {
  // Find the index of the checkbox that triggered the event
  for (size_t i = 0; i < options.size(); i++) {
    if (cbOption[i] == object) {
      bool currentState = settingsController.IsWidgetOn(options[i].widget);
      settingsController.SetWidget(options[i].widget, !currentState);
      break;
    }
  }

  // Update checkbox according to current widgets.
  auto modes = settingsController.GetWidgets();
  for (size_t i = 0; i < options.size(); ++i) {
    lv_checkbox_set_checked(cbOption[i], modes[i]);
  }
}
