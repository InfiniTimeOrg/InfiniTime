#include "displayapp/screens/FlashLight.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

FlashLight::FlashLight(Pinetime::Applications::DisplayApp* app,
                       System::SystemTask& systemTask,
                       Controllers::BrightnessController& brightnessController)
  : Screen(app), systemTask {systemTask}, brightnessController {brightnessController} {

  flashLight = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(flashLight, Symbols::highlight);
  lv_obj_align(flashLight, nullptr, LV_ALIGN_CENTER, 0, 0);

  brightnessController.Backup();

  SetBrightness();

  SetColors();

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

FlashLight::~FlashLight() {
  lv_obj_clean(lv_scr_act());
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  brightnessController.Restore();
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void FlashLight::SetColors() {
  if (isOn) {
    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_text_color(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  } else {
    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    if (highBrightness) {
      lv_obj_set_style_local_text_color(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    } else {
      lv_obj_set_style_local_text_color(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    }
  }
}

void FlashLight::SetBrightness() {
  if (highBrightness) {
    if (isOn) {
      brightnessController.Set(Controllers::BrightnessController::Levels::High);
    } else {
      brightnessController.Restore();
    }
  } else {
    brightnessController.Set(Controllers::BrightnessController::Levels::Low);
  }
}

bool FlashLight::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (event == TouchEvents::SwipeLeft || event == TouchEvents::SwipeRight) {
    highBrightness = !highBrightness;
    SetBrightness();
    if (!isOn) {
      SetColors();
    }
    return true;
  }
  if (event == TouchEvents::Tap) {
    // When launching the app, one tap event gets erronously handled. Ignore it in app for now.
    if (!firstTapDone) {
      firstTapDone = true;
      return true;
    }
    isOn = !isOn;
    SetColors();
    SetBrightness();
    return true;
  }

  return false;
}
