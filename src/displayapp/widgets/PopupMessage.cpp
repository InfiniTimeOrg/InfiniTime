#include "displayapp/widgets/PopupMessage.h"
#include "displayapp/InfiniTimeTheme.h"
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Applications::Widgets;

void PopupMessage::Create() {
  popup = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(popup, 90, 90);
  lv_obj_align(popup, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_color(popup, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Colors::bg);
  lv_obj_set_style_local_bg_opa(popup, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
  lv_obj_t* lockBody = lv_obj_create(popup, nullptr);
  lv_obj_set_size(lockBody, 55, 50);
  lv_obj_align(lockBody, popup, LV_ALIGN_CENTER, 0, 10);

  lv_obj_set_style_local_bg_color(lockBody, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_bg_opa(lockBody, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_border_color(lockBody, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_border_width(lockBody, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 22);
  lv_obj_set_style_local_border_side(lockBody, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
  lv_obj_set_style_local_border_opa(lockBody, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);

  lv_obj_t* lockTop = lv_obj_create(popup, nullptr);
  lv_obj_set_size(lockTop, 30, 35);
  lv_obj_align(lockTop, popup, LV_ALIGN_CENTER, 0, -20);
  lv_obj_set_style_local_bg_color(lockTop, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_bg_opa(lockTop, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_border_color(lockTop, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_border_width(lockTop, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 6);
  lv_obj_set_style_local_border_side(lockTop, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
  lv_obj_set_style_local_border_opa(lockTop, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);

  lv_obj_set_hidden(popup, isHidden);
}

void PopupMessage::SetHidden(const bool hidden) {
  if (isHidden == hidden) {
    return;
  }
  isHidden = hidden;
  // create/delete on demand
  if (popup == nullptr && !isHidden) {
    Create();
  } else if (popup != nullptr) {
    lv_obj_del(popup);
    popup = nullptr;
  }
}

bool PopupMessage::IsHidden() const {
  return isHidden;
}
