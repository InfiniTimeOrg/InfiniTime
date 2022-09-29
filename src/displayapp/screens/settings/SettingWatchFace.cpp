#include "displayapp/screens/settings/SettingWatchFace.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/CheckboxList.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* settings = static_cast<SettingWatchFace*>(obj->user_data);
    settings->UpdateSelected(obj, event);
  }
}

auto SettingWatchFace::CreateScreenList() {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screens;
  for (uint8_t i = 0; i < screens.size(); i++) {
    screens[i] = [this, i]() -> std::unique_ptr<Screen> { return CreateScreen(i); };
  }
  return screens;
}

std::unique_ptr<Screen> SettingWatchFace::CreateScreen(uint8_t screenIdx) {
  /* Container */
  lv_obj_t* container = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_pos(container, 0, 0);

  /* Title... */
  lv_obj_t* title = lv_label_create(container, nullptr);
  lv_label_set_text_static(title, this.title);
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, container, LV_ALIGN_IN_TOP_MID, 10, 15);

  /* ...with icon */
  lv_obj_t* icon = lv_label_create(container, nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, this.icon);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  /* Watchface option list */
  lv_obj_t* list = lv_cont_create(container, nullptr);

  lv_obj_set_style_local_bg_opa(list, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(list, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(list, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(list, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(list, 10, 60);
  lv_obj_set_width(list, LV_HOR_RES - 20);
  lv_obj_set_height(list, LV_VER_RES - 50);
  lv_cont_set_layout(list, LV_LAYOUT_COLUMN_LEFT);

  for (uint8_t i = 0; i < optionsPerScreen ; i++) {
    uint8_t optionIdx = screenIdx * optionsPerScreen + i;
    if (optionIdx < nOptions) {
      lv_obj_t* checkbox = lv_checkbox_create(list, nullptr);
      lv_checkbox_set_text(checkbox, options[optionIdx]);
      checkbox->user_data = this;
      lv_obj_set_event_cb(checkbox, event_handler);
      SetRadioButtonStyle(checkbox);
      if (optionIdx == settingsController.GetClockFace()) {
        lv_checkbox_set_checked(checkbox, true);
      }
      checkboxes[i] = checkbox;
    } else {
      checkboxes[i] = nullptr;
    }
  }

  return std::make_unique<Screens::Container>(app, container, screenIdx, nScreens, true);
}

SettingWatchFace::SettingWatchFace(DisplayApp* app, Controllers::Settings& settingsController)
  : Screen(app),
    settingsController {settingsController},
    screens {app, 0, CreateScreenList(), Screens::ScreenListModes::RightLeft} { }

SettingWatchFace::~SettingWatchFace() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

bool SettingWatchFace::OnTouchEvent(Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

void SettingWatchFace::UpdateSelected(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    uint8_t screenIdx = screens.getScreenIndex(); 
    for (uint8_t i = 0; i < optionsPerScreen && checkboxes[i]; i++) {
      if (obj == checkboxes[i]) {
        lv_checkbox_set_checked(checkboxes[i], true);
        settingsController.SetClockFace(screenIdx * optionsPerScreen + i);
      } else {
        lv_checkbox_set_checked(checkboxes[i], false);
      }
    }
  }
}
