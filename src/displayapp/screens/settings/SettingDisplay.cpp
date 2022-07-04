#include "displayapp/screens/settings/SettingDisplay.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/Messages.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void IncreaseBtnHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingDisplay*>(obj->user_data);
    if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
      screen->Increase();
    }
  }
  void DecreaseBtnHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<SettingDisplay*>(obj->user_data);
    if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
      screen->Decrease();
    }
  }
}

constexpr std::array<uint16_t, 6> SettingDisplay::options;

SettingDisplay::SettingDisplay(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController)
  : Screen(app), settingsController {settingsController}, setTimeout {settingsController.GetScreenTimeOut()} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Display timeout");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::sun);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  timeoutLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(timeoutLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(timeoutLabel, "00");
  lv_label_set_align(timeoutLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_long_mode(timeoutLabel, LV_LABEL_LONG_CROP);
  static constexpr int horizOffset = -LV_HOR_RES_MAX / 4;
  lv_obj_align(timeoutLabel, nullptr, LV_ALIGN_CENTER, horizOffset, 0);

  lv_label_set_text_fmt(timeoutLabel, "%d", setTimeout / 1000);

  lv_obj_t* secondsLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(secondsLabel, "seconds");
  lv_obj_align(secondsLabel, timeoutLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  static constexpr int btnWidth = LV_HOR_RES_MAX / 2;
  static constexpr lv_coord_t btnHeight = 90;

  decreaseBtn = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(decreaseBtn, btnWidth, btnHeight);
  lv_obj_align(decreaseBtn, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  decreaseBtn->user_data = this;
  lv_obj_set_event_cb(decreaseBtn, DecreaseBtnHandler);

  lv_obj_t* decreaseLabel = lv_label_create(decreaseBtn, nullptr);
  lv_obj_set_style_local_text_font(decreaseLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(decreaseLabel, "-");
  lv_obj_align(decreaseLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

  increaseBtn = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(increaseBtn, btnWidth, btnHeight);
  lv_obj_align(increaseBtn, decreaseBtn, LV_ALIGN_OUT_TOP_MID, 0, -10);
  increaseBtn->user_data = this;
  lv_obj_set_event_cb(increaseBtn, IncreaseBtnHandler);

  lv_obj_t* increaseLabel = lv_label_create(increaseBtn, nullptr);
  lv_obj_set_style_local_text_font(increaseLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(increaseLabel, "+");
  lv_obj_align(increaseLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
}

SettingDisplay::~SettingDisplay() {
  lv_obj_clean(lv_scr_act());
  settingsController.SetScreenTimeOut(setTimeout);
  app->PushMessage(Applications::Display::Messages::UpdateTimeOut);
  settingsController.SaveSettings();
}

void SettingDisplay::Increase() {
  for (uint16_t option : options) {
    if (option > setTimeout) {
      setTimeout = option;
      break;
    }
  }
  lv_label_set_text_fmt(timeoutLabel, "%d", setTimeout / 1000);
}

void SettingDisplay::Decrease() {
  for (size_t i = 0; i < options.size() - 1; i++) {
    if (options[i + 1] >= setTimeout) {
      setTimeout = options[i];
      break;
    }
  }
  lv_label_set_text_fmt(timeoutLabel, "%d", setTimeout / 1000);
}
