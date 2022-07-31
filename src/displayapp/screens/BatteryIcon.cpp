#include "displayapp/screens/BatteryIcon.h"
#include <cstdint>
#include "displayapp/screens/Symbols.h"
#include "displayapp/icons/battery/batteryicon.c"

using namespace Pinetime::Applications::Screens;

void BatteryIcon::Create(lv_obj_t* parent) {

  batteryContainer = lv_cont_create(parent, nullptr);
  lv_cont_set_layout(batteryContainer, LV_LAYOUT_ROW_MID);
  lv_cont_set_fit(batteryContainer, LV_FIT_TIGHT);
  lv_obj_set_style_local_bg_color(batteryContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_pad_inner(batteryContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_hor(batteryContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  batteryImg = lv_img_create(batteryContainer, nullptr);
  lv_img_set_src(batteryImg, &batteryicon);
  lv_obj_set_style_local_image_recolor(batteryImg, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  batteryJuice = lv_obj_create(batteryImg, nullptr);
  lv_obj_set_width(batteryJuice, 8);
  lv_obj_align(batteryJuice, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -2, -2);
  lv_obj_set_style_local_radius(batteryJuice, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

  batteryPercentageText = lv_label_create(batteryContainer, nullptr);
  lv_label_set_text(batteryPercentageText, "");
  lv_obj_align(batteryPercentageText, batteryContainer, LV_ALIGN_IN_RIGHT_MID, 0, 0);
}

lv_obj_t* BatteryIcon::GetObject() {
  return batteryContainer;
}

void BatteryIcon::SetBatteryPercentage(uint8_t percentage, bool show_percentage) {
  lv_obj_set_height(batteryJuice, percentage * 14 / 100);
  lv_obj_realign(batteryJuice);
  if (show_percentage) {
    lv_label_set_text_fmt(batteryPercentageText, "%02i%%", percentage);
  } else {
    lv_label_set_text(batteryPercentageText, "");
  }
  lv_obj_realign(batteryPercentageText);
  lv_obj_realign(batteryContainer);
}

void BatteryIcon::SetColor(lv_color_t color) {
  lv_obj_set_style_local_image_recolor(batteryImg, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, color);
  lv_obj_set_style_local_image_recolor_opa(batteryImg, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_bg_color(batteryJuice, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color);
}

const char* BatteryIcon::GetPlugIcon(bool isCharging) {
  if (isCharging) {
    return Symbols::plug;
  }
  return "";
}
