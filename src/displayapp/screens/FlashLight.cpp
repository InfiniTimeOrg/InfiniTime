#include "displayapp/screens/FlashLight.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
      auto* screen = static_cast<FlashLight*>(obj->user_data);
      screen->Toggle();
    }
  }
}

FlashLight::FlashLight(System::SystemTask& systemTask, Controllers::BrightnessController& brightnessController)
  : systemTask {systemTask}, brightnessController {brightnessController} {

  brightnessController.Set(Controllers::BrightnessController::Levels::Low);

  flashLight = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(flashLight, Symbols::flashlight);
  lv_obj_align(flashLight, nullptr, LV_ALIGN_CENTER, 0, 0);

  for (auto& indicator : indicators) {
    indicator = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_size(indicator, 15, 10);
    lv_obj_set_style_local_border_width(indicator, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
  }

  lv_obj_align(indicators[1], flashLight, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
  lv_obj_align(indicators[0], indicators[1], LV_ALIGN_OUT_LEFT_MID, -8, 0);
  lv_obj_align(indicators[2], indicators[1], LV_ALIGN_OUT_RIGHT_MID, 8, 0);

  SetIndicators();
  SetColors();

  backgroundAction = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundAction, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundAction, 240, 240);
  lv_obj_set_pos(backgroundAction, 0, 0);
  lv_label_set_text_static(backgroundAction, "");
  lv_obj_set_click(backgroundAction, true);
  backgroundAction->user_data = this;
  lv_obj_set_event_cb(backgroundAction, EventHandler);

  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

FlashLight::~FlashLight() {
  lv_obj_clean(lv_scr_act());
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void FlashLight::SetColors() {
  lv_color_t bgColor = isOn ? LV_COLOR_WHITE : LV_COLOR_BLACK;
  lv_color_t fgColor = isOn ? Colors::lightGray : LV_COLOR_WHITE;

  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, bgColor);
  lv_obj_set_style_local_text_color(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fgColor);
  for (auto& indicator : indicators) {
    lv_obj_set_style_local_bg_color(indicator, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, fgColor);
    lv_obj_set_style_local_bg_color(indicator, LV_OBJ_PART_MAIN, LV_STATE_DISABLED, bgColor);
    lv_obj_set_style_local_border_color(indicator, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, fgColor);
  }
}

void FlashLight::SetIndicators() {
  using namespace Pinetime::Controllers;

  if (brightnessLevel == BrightnessController::Levels::High) {
    lv_obj_set_state(indicators[1], LV_STATE_DEFAULT);
    lv_obj_set_state(indicators[2], LV_STATE_DEFAULT);
  } else if (brightnessLevel == BrightnessController::Levels::Medium) {
    lv_obj_set_state(indicators[1], LV_STATE_DEFAULT);
    lv_obj_set_state(indicators[2], LV_STATE_DISABLED);
  } else {
    lv_obj_set_state(indicators[1], LV_STATE_DISABLED);
    lv_obj_set_state(indicators[2], LV_STATE_DISABLED);
  }
}

void FlashLight::Toggle() {
  isOn = !isOn;
  SetColors();
  if (isOn) {
    brightnessController.Set(brightnessLevel);
  } else {
    brightnessController.Set(Controllers::BrightnessController::Levels::Low);
  }
}

bool FlashLight::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  using namespace Pinetime::Controllers;

  auto SetState = [this]() {
    if (isOn) {
      brightnessController.Set(brightnessLevel);
    }
    SetIndicators();
  };

  if (event == TouchEvents::SwipeLeft) {
    if (brightnessLevel == BrightnessController::Levels::High) {
      brightnessLevel = BrightnessController::Levels::Medium;
      SetState();
    } else if (brightnessLevel == BrightnessController::Levels::Medium) {
      brightnessLevel = BrightnessController::Levels::Low;
      SetState();
    }
    return true;
  }
  if (event == TouchEvents::SwipeRight) {
    if (brightnessLevel == BrightnessController::Levels::Low) {
      brightnessLevel = BrightnessController::Levels::Medium;
      SetState();
    } else if (brightnessLevel == BrightnessController::Levels::Medium) {
      brightnessLevel = BrightnessController::Levels::High;
      SetState();
    }
    return true;
  }

  return false;
}
