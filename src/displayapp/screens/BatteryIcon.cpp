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

void BatteryIcon::SetBatteryPercentage(uint8_t percentage) {
  lv_obj_set_height(batteryJuice, percentage * 14 / 100);
  lv_obj_realign(batteryJuice);

  // Function to convert a percentage value from 0 to 100 to a value from 0 to 180 with non-linear adjustments
  double yeet(double percentage) {
      double val = percentage / 100.0;
  
      if (val >= 0.001) { // Adjusted for a small tolerance to avoid division by zero
          if (val > 0.45 && val < 0.8) {
              return 0.33333 * 180;
          } else if (val < 0.5) {
              return (val - 0.001) * 0.96 * 180;
          } else {
              return (val - 0.4) * 0.85 * 180;
          }
      } else {
          return 0;
      }
  }
  SetColor( lv_color_hsv_to_rgb( ( percentage/2 ) ,100,100) );
  
//  if (colorOnLowBattery) {
//    static constexpr int lowBatteryThreshold = 15;
//    static constexpr int criticalBatteryThreshold = 5;
//    if (percentage > lowBatteryThreshold) {
//      SetColor(LV_COLOR_WHITE);
//    } else if (percentage > criticalBatteryThreshold) {
//      SetColor(LV_COLOR_ORANGE);
//   } else {
//      SetColor(Colors::deepOrange);
//    }
//  }
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
