#include "FlashLight.h"
#include "../DisplayApp.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    FlashLight* screen = static_cast<FlashLight*>(obj->user_data);
    screen->OnClickEvent(obj, event);
  }
}

FlashLight::FlashLight(Pinetime::Applications::DisplayApp* app,
                       System::SystemTask& systemTask,
                       Controllers::BrightnessController& brightness)
  : Screen(app),
    systemTask {systemTask},
    brightness {brightness}

{
  brightness.Backup();
  brightness.Set(Controllers::BrightnessController::Levels::High);
  // Set the background
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));

  flashLight = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  lv_obj_set_style_local_text_font(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(flashLight, Symbols::highlight);
  lv_obj_align(flashLight, NULL, LV_ALIGN_CENTER, 0, 0);

  backgroundAction = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundAction, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundAction, 240, 240);
  lv_obj_set_pos(backgroundAction, 0, 0);
  lv_label_set_text(backgroundAction, "");
  lv_obj_set_click(backgroundAction, true);
  backgroundAction->user_data = this;
  lv_obj_set_event_cb(backgroundAction, event_handler);

  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

FlashLight::~FlashLight() {
  lv_obj_clean(lv_scr_act());
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
  brightness.Restore();
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void FlashLight::OnClickEvent(lv_obj_t* obj, lv_event_t event) {
  if (obj == backgroundAction) {
    if (event == LV_EVENT_CLICKED) {
      isOn = !isOn;

      if (isOn) {
        lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
        lv_obj_set_style_local_text_color(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
      } else {
        lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));
        lv_obj_set_style_local_text_color(flashLight, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
      }
    }
  }
}

bool FlashLight::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return false;
}
