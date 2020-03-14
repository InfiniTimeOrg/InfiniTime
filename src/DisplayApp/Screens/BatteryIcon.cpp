#include "BatteryIcon.h"

using namespace Pinetime::Applications::Screens;


extern lv_img_dsc_t os_battery_error;
extern lv_img_dsc_t os_batterycharging_100;
extern lv_img_dsc_t os_batterycharging_090;
extern lv_img_dsc_t os_batterycharging_080;
extern lv_img_dsc_t os_batterycharging_070;
extern lv_img_dsc_t os_batterycharging_060;
extern lv_img_dsc_t os_batterycharging_050;
extern lv_img_dsc_t os_batterycharging_040;
extern lv_img_dsc_t os_batterycharging_030;
extern lv_img_dsc_t os_batterycharging_020;
extern lv_img_dsc_t os_batterycharging_010;
extern lv_img_dsc_t os_batterycharging_005;

extern lv_img_dsc_t os_battery_100;
extern lv_img_dsc_t os_battery_090;
extern lv_img_dsc_t os_battery_080;
extern lv_img_dsc_t os_battery_070;
extern lv_img_dsc_t os_battery_060;
extern lv_img_dsc_t os_battery_050;
extern lv_img_dsc_t os_battery_040;
extern lv_img_dsc_t os_battery_030;
extern lv_img_dsc_t os_battery_020;
extern lv_img_dsc_t os_battery_010;
extern lv_img_dsc_t os_battery_005;


lv_img_dsc_t *BatteryIcon::GetIcon(bool isCharging, float batteryPercent) {
  if(isCharging) {
    if(batteryPercent > 90.0f) return &os_batterycharging_100;
    else if(batteryPercent > 80.0f) return &os_batterycharging_090;
    else if(batteryPercent > 70.0f) return &os_batterycharging_080;
    else if(batteryPercent > 60.0f) return &os_batterycharging_070;
    else if(batteryPercent > 50.0f) return &os_batterycharging_060;
    else if(batteryPercent > 40.0f) return &os_batterycharging_050;
    else if(batteryPercent > 30.0f) return &os_batterycharging_040;
    else if(batteryPercent > 20.0f) return &os_batterycharging_030;
    else if(batteryPercent > 10.0f) return &os_batterycharging_020;
    else if(batteryPercent > 5.0f) return &os_batterycharging_010;
    else return &os_batterycharging_005;
  } else {
    if(batteryPercent > 90.0f) return &os_battery_100;
    else if(batteryPercent > 80.0f) return &os_battery_090;
    else if(batteryPercent > 70.0f) return &os_battery_080;
    else if(batteryPercent > 60.0f) return &os_battery_070;
    else if(batteryPercent > 50.0f) return &os_battery_060;
    else if(batteryPercent > 40.0f) return &os_battery_050;
    else if(batteryPercent > 30.0f) return &os_battery_040;
    else if(batteryPercent > 20.0f) return &os_battery_030;
    else if(batteryPercent > 10.0f) return &os_battery_020;
    else if(batteryPercent > 5.0f) return &os_battery_010;
    else return &os_battery_005;
  }
}

lv_img_dsc_t *BatteryIcon::GetUnknownIcon() {
  return &os_battery_error;
}
