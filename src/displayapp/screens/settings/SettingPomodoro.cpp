#include "displayapp/screens/settings/SettingPomodoro.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/TouchEvents.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingPomodoro* screen = static_cast<SettingPomodoro*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingPomodoro::SettingPomodoro(Pinetime::Controllers::Settings& settingsController)
  : settingsController {settingsController}, currentSetting(0) {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_pos(container1, 30, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 50);
  lv_obj_set_height(container1, LV_VER_RES - 60);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(title, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 1, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::clock);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  settingValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(settingValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_align(settingValue, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(settingValue, lv_scr_act(), LV_ALIGN_CENTER, 0, -20);

  static constexpr uint8_t btnWidth = 115;
  static constexpr uint8_t btnHeight = 80;

  btnPlus = lv_btn_create(lv_scr_act(), nullptr);
  btnPlus->user_data = this;
  lv_obj_set_size(btnPlus, btnWidth, btnHeight);
  lv_obj_align(btnPlus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_local_bg_color(btnPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblPlus = lv_label_create(btnPlus, nullptr);
  lv_obj_set_style_local_text_font(lblPlus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblPlus, "+");
  lv_obj_set_event_cb(btnPlus, event_handler);

  btnMinus = lv_btn_create(lv_scr_act(), nullptr);
  btnMinus->user_data = this;
  lv_obj_set_size(btnMinus, btnWidth, btnHeight);
  lv_obj_set_event_cb(btnMinus, event_handler);
  lv_obj_align(btnMinus, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_color(btnMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_t* lblMinus = lv_label_create(btnMinus, nullptr);
  lv_obj_set_style_local_text_font(lblMinus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(lblMinus, "-");

  // Update display with current setting
  UpdateDisplay();
}

SettingPomodoro::~SettingPomodoro() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void SettingPomodoro::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event != LV_EVENT_SHORT_CLICKED && event != LV_EVENT_LONG_PRESSED && event != LV_EVENT_LONG_PRESSED_REPEAT) {
    return;
  }

  if (object == btnPlus) {
    switch (currentSetting) {
      case 0: // Work Duration
        if (settingsController.GetPomodoroWorkDuration() == Controllers::Settings::PomodoroWorkDuration::Minutes25) {
          settingsController.SetPomodoroWorkDuration(Controllers::Settings::PomodoroWorkDuration::Minutes50);
          // Automatically set break durations for 50-minute work sessions
          settingsController.SetPomodoroShortBreakMinutes(10);
          settingsController.SetPomodoroLongBreakMinutes(20);
        }
        break;
      case 1: // Sessions
      {
        uint8_t currentValue = settingsController.GetPomodoroSessionsBeforeLongBreak();
        if (currentValue < 8) {
          settingsController.SetPomodoroSessionsBeforeLongBreak(currentValue + 1);
        }
      } break;
    }
  } else if (object == btnMinus) {
    switch (currentSetting) {
      case 0: // Work Duration
        if (settingsController.GetPomodoroWorkDuration() == Controllers::Settings::PomodoroWorkDuration::Minutes50) {
          settingsController.SetPomodoroWorkDuration(Controllers::Settings::PomodoroWorkDuration::Minutes25);
          // Automatically set break durations for 25-minute work sessions
          settingsController.SetPomodoroShortBreakMinutes(5);
          settingsController.SetPomodoroLongBreakMinutes(10);
        }
        break;
      case 1: // Sessions
      {
        uint8_t currentValue = settingsController.GetPomodoroSessionsBeforeLongBreak();
        if (currentValue > 2) {
          settingsController.SetPomodoroSessionsBeforeLongBreak(currentValue - 1);
        }
      } break;
    }
  }

  UpdateDisplay();
}

bool SettingPomodoro::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeLeft:
      currentSetting = (currentSetting + 1) % 2;
      UpdateDisplay();
      return true;
    case TouchEvents::SwipeRight:
      currentSetting = (currentSetting + 1) % 2; // Toggle between 0 and 1
      UpdateDisplay();
      return true;
    default:
      return false;
  }
}

void SettingPomodoro::UpdateDisplay() {
  switch (currentSetting) {
    case 0: // Work Duration
      lv_label_set_text_static(title, "Work duration");
      if (settingsController.GetPomodoroWorkDuration() == Controllers::Settings::PomodoroWorkDuration::Minutes25) {
        lv_label_set_text_static(settingValue, "25 min");
      } else {
        lv_label_set_text_static(settingValue, "50 min");
      }
      break;
    case 1: // Sessions
      lv_label_set_text_static(title, "Sessions before long break");
      lv_label_set_text_fmt(settingValue, "%d", settingsController.GetPomodoroSessionsBeforeLongBreak());
      break;
  }

  lv_obj_realign(settingValue);
}