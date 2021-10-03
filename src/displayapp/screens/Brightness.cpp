#include "Brightness.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

void slider_event_cb(lv_obj_t* slider, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    auto* brightnessSlider = static_cast<Brightness*>(slider->user_data);
    brightnessSlider->OnValueChanged();
  }
}

Brightness::Brightness(Pinetime::Applications::DisplayApp* app, Controllers::BrightnessController& brightness)
  : Screen(app), brightness {brightness} {
  slider = lv_slider_create(lv_scr_act(), nullptr);
  lv_obj_set_user_data(slider, this);
  lv_obj_set_width(slider, LV_DPI * 2);
  lv_obj_align(slider, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_event_cb(slider, slider_event_cb);
  lv_slider_set_range(slider, 0, 2);
  lv_slider_set_value(slider, LevelToInt(brightness.Level()), LV_ANIM_OFF);

  slider_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(slider_label, LevelToString(brightness.Level()));
  lv_obj_set_auto_realign(slider_label, true);
  lv_obj_align(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
}

Brightness::~Brightness() {
  lv_obj_clean(lv_scr_act());
}

const char* Brightness::LevelToString(Pinetime::Controllers::BrightnessController::Levels level) {
  switch (level) {
    case Pinetime::Controllers::BrightnessController::Levels::Off:
      return "Off";
    case Pinetime::Controllers::BrightnessController::Levels::Low:
      return "Low";
    case Pinetime::Controllers::BrightnessController::Levels::Medium:
      return "Medium";
    case Pinetime::Controllers::BrightnessController::Levels::High:
      return "High";
    default:
      return "???";
  }
}

void Brightness::OnValueChanged() {
  SetValue(lv_slider_get_value(slider));
}

void Brightness::SetValue(uint8_t value) {
  switch (value) {
    case 0:
      brightness.Set(Controllers::BrightnessController::Levels::Low);
      break;
    case 1:
      brightness.Set(Controllers::BrightnessController::Levels::Medium);
      break;
    case 2:
      brightness.Set(Controllers::BrightnessController::Levels::High);
      break;
  }
  lv_label_set_text(slider_label, LevelToString(brightness.Level()));
}

uint8_t Brightness::LevelToInt(Pinetime::Controllers::BrightnessController::Levels level) {
  switch (level) {
    case Pinetime::Controllers::BrightnessController::Levels::Off:
      return 0;
    case Pinetime::Controllers::BrightnessController::Levels::Low:
      return 0;
    case Pinetime::Controllers::BrightnessController::Levels::Medium:
      return 1;
    case Pinetime::Controllers::BrightnessController::Levels::High:
      return 2;
    default:
      return 0;
  }
}

bool Brightness::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeLeft:
      brightness.Lower();
      if (brightness.Level() == Pinetime::Controllers::BrightnessController::Levels::Off) {
        brightness.Set(Controllers::BrightnessController::Levels::Low);
      }
      SetValue();
      return true;
    case TouchEvents::SwipeRight:
      brightness.Higher();
      SetValue();
      return true;
    default:
      return false;
  }
}

void Brightness::SetValue() {
  lv_slider_set_value(slider, LevelToInt(brightness.Level()), LV_ANIM_OFF);
  lv_label_set_text(slider_label, LevelToString(brightness.Level()));
}
