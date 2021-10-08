#include "FlashLight.h"
#include "../DisplayApp.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

FlashLight::FlashLight(Pinetime::Applications::DisplayApp* app,
                       System::SystemTask& systemTask,
                       Controllers::BrightnessController& brightness,
                       Pinetime::Controllers::Settings &settingsController)
  : Screen(app),
    systemTask {systemTask},
    brightness {brightness},
    settingsController {settingsController}

{
  brightness.Backup();
  brightness.Set(Controllers::BrightnessController::Levels::High);
  currentColorIndex = settingsController.getLastTorchColorIndex();
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Convert(torchColors[currentColorIndex]));
  backgroundAction = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundAction, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundAction, 240, 240);
  lv_obj_set_pos(backgroundAction, 0, 0);
  lv_label_set_text(backgroundAction, "");
  lv_obj_set_click(backgroundAction, true);
  backgroundAction->user_data = this;

  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

FlashLight::~FlashLight() {
  settingsController.setLastTorchColorIndex(currentColorIndex);
  lv_obj_clean(lv_scr_act());
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Convert(Applications::Colors::Black));
  brightness.Restore();
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

bool FlashLight::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeLeft:
      currentColorIndex--;
      if (currentColorIndex <= 0) {
        currentColorIndex = 11;
      }
      lv_obj_set_style_local_bg_color(lv_scr_act(), LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(torchColors[currentColorIndex]));
      break;
    case TouchEvents::SwipeRight:
      currentColorIndex++;
      if (currentColorIndex >= 11) {
        currentColorIndex = 0;
      }
      lv_obj_set_style_local_bg_color(lv_scr_act(), LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(torchColors[currentColorIndex]));
      break;
    case TouchEvents::SwipeUp:
      brightness.Higher();
      break;
    case TouchEvents::SwipeDown:
      brightness.Lower();
      break;
    case TouchEvents::DoubleTap:
      if (brightness.Level() == Pinetime::Controllers::BrightnessController::Levels::Off) {
        brightness.Set(Pinetime::Controllers::BrightnessController::Levels::Medium);
      } else {
        brightness.Set(Pinetime::Controllers::BrightnessController::Levels::Off);
      }
      break;
    case TouchEvents::LongTap:
      currentColorIndex = 0;
      lv_obj_set_style_local_bg_color(lv_scr_act(), LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(torchColors[currentColorIndex]));
      break;      
    default:
      break;
  }
  return false;
}
