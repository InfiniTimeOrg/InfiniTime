#include "displayapp/screens/settings/SettingWakeUp.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Styles.h"

using namespace Pinetime::Applications::Screens;

constexpr std::array<SettingWakeUp::Option, SettingWakeUp::numOptions> SettingWakeUp::options;

auto SettingWakeUp::CreateScreenList() {
  std::array<std::function<std::unique_ptr<Screen>()>, nScreens> screens;
  for (size_t i = 0; i < screens.size(); i++) {
    screens[i] = [this, i]() -> std::unique_ptr<Screen> {
      return CreateScreen(i);
    };
  }
  return screens;
}

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingWakeUp*>(obj->user_data);
    if (event == LV_EVENT_VALUE_CHANGED) {
      screen->UpdateSelected(obj);
    }
  }
}

SettingWakeUp::SettingWakeUp(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : settingsController {settingsController}, screens {app, 0, CreateScreenList(), Screens::ScreenListModes::UpDown} {
}

SettingWakeUp::~SettingWakeUp() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

std::unique_ptr<Screen> SettingWakeUp::CreateScreen(size_t screenNum) {
  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 35);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 20);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Wake Up");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 15, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::eye);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  // cleanup any old pointers
  cbOption.fill(nullptr);

  // only loop as far as the list size aĺlows
  unsigned int loopMax = screenNum * optionsPerScreen + optionsPerScreen;
  if (loopMax > options.size()) {
    loopMax = options.size();
  }

  for (unsigned int i = screenNum * optionsPerScreen; i < loopMax; i++) {
    cbOption[i] = lv_checkbox_create(container1, nullptr);
    lv_checkbox_set_text(cbOption[i], options[i].name);
    if (settingsController.isWakeUpModeOn(options[i].wakeUpMode)) {
      lv_checkbox_set_checked(cbOption[i], true);
    }
    cbOption[i]->user_data = this;
    lv_obj_set_event_cb(cbOption[i], event_handler);
  }
  return std::make_unique<Screens::Page>(screenNum, nScreens, container1);
}

bool SettingWakeUp::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}

void SettingWakeUp::UpdateSelected(lv_obj_t* object) {
  // Find the index of the checkbox that triggered the event
  for (size_t i = 0; i < options.size(); i++) {
    if (cbOption[i] == object) {
      bool currentState = settingsController.isWakeUpModeOn(options[i].wakeUpMode);
      settingsController.setWakeUpMode(options[i].wakeUpMode, !currentState);
      break;
    }
  }

  // Update checkbox according to current wakeup modes.
  // This is needed because we can have extra logic when setting or unsetting wakeup modes,
  // for example, when setting SingleTap, DoubleTap is unset and vice versa.
  auto modes = settingsController.getWakeUpModes();
  for (size_t i = 0; i < options.size(); ++i) {
    if (cbOption[i]) {
      lv_checkbox_set_checked(cbOption[i], modes[i]);
    }
  }
}
