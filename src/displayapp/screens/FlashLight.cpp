#include "FlashLight.h"
#include "../DisplayApp.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_event_t* event) {
    FlashLight* screen = static_cast<FlashLight*>(lv_event_get_user_data(event));
    screen->OnClickEvent(lv_event_get_target(event), event);
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
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

  flashLight = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(flashLight, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(flashLight, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(flashLight, Symbols::highlight);
  lv_obj_align(flashLight, LV_ALIGN_CENTER, 0, 0);

  backgroundAction = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundAction, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundAction, 240, 240);
  lv_obj_set_pos(backgroundAction, 0, 0);
  lv_label_set_text(backgroundAction, "");
  lv_obj_add_flag(backgroundAction, LV_OBJ_FLAG_CLICKABLE);
  backgroundAction->user_data = this;
  lv_obj_add_event_cb(backgroundAction, event_handler, LV_EVENT_ALL, backgroundAction->user_data);

  systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

FlashLight::~FlashLight() {
  lv_obj_clean(lv_scr_act());
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  brightness.Restore();
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void FlashLight::OnClickEvent(lv_obj_t* obj, lv_event_t* event) {
  if (obj == backgroundAction) {
    if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
      isOn = !isOn;

      if (isOn) {
        lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(flashLight, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
      } else {
        lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(flashLight, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
      }
    }
  }
}

bool FlashLight::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return false;
}
