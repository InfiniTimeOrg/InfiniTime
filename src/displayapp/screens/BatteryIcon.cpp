#include "displayapp/screens/BatteryIcon.h"
#include <cstdint>
#include "displayapp/screens/Symbols.h"
#include "displayapp/icons/battery/batteryicon.c"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

BatteryIcon::BatteryIcon(bool colorOnLowBattery) : colorOnLowBattery {colorOnLowBattery} {};

void BatteryIcon::Create(lv_obj_t* parent) {
  batteryImg = lv_img_create(parent, nullptr);
  lv_img_set_src(batteryImg, &batteryicon);
  lv_obj_set_style_local_image_recolor(batteryImg, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  batteryJuice = lv_obj_create(batteryImg, nullptr);
  lv_obj_set_width(batteryJuice, 8);
  lv_obj_align(batteryJuice, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -2, -2);
  lv_obj_set_style_local_radius(batteryJuice, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
}

lv_obj_t* BatteryIcon::GetObject() {
  return batteryImg;
}

// Function to convert a percentage value from 0 to 100 to a value from 0 to 180 with non-linear adjustments
double BatteryIcon::ColorRamp(double percentage) {
    double val = percentage / 100.0;

    if (val >= 0.05) { 
        if (val >  0.25 && val < 0.75) {
            return 0.33333 * 360;
        } else if (val >= 0.9) {
            return 0.5 * 360;
        } else if (val < 0.5) {
            return (val - 0.05) * 1.61 * 360;
        } else {
            return (val - 0.35) * 0.85 * 360;
        }
    } else {
        return 0;
    }
}

void BatteryIcon::SetBatteryPercentage(uint8_t percentage) {
  lv_obj_set_height(batteryJuice, percentage * 14 / 100);
  lv_obj_realign(batteryJuice);

  // SetColor( lv_color_hsv_to_rgb( percentage/2 ,100,100) );
  SetColor( lv_color_hsv_to_rgb( ColorRamp(percentage) ,100,100) );
}

void BatteryIcon::SetColor(lv_color_t color) {
  lv_obj_set_style_local_image_recolor(batteryImg, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, color);
  lv_obj_set_style_local_image_recolor_opa(batteryImg, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_obj_set_style_local_bg_color(batteryJuice, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, color);
}

const char* BatteryIcon::GetPlugIcon(bool isCharging) {
  if (isCharging)
    return Symbols::plug;
  else
    return "";
}
