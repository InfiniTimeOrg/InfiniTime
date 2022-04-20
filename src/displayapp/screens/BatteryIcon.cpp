#include "displayapp/screens/BatteryIcon.h"
#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

const char* BatteryIcon::GetBatteryIcon(uint8_t batteryPercent) {
  if (batteryPercent > 87) {
    return Symbols::batteryFull;
  }
  if (batteryPercent > 62) {
    return Symbols::batteryThreeQuarter;
  }
  if (batteryPercent > 37) {
    return Symbols::batteryHalf;
  }
  if (batteryPercent > 12) {
    return Symbols::batteryOneQuarter;
  }
  return Symbols::batteryEmpty;
}

const lv_color_t BatteryIcon::GetBatteryColor(uint8_t batteryPercent) {
  if (batteryPercent > 75) {
    return LV_COLOR_GREEN;
  }
  if (batteryPercent > 50) {
    return LV_COLOR_YELLOW;
  }
  if (batteryPercent > 25) {
    return LV_COLOR_ORANGE;
  }
  return LV_COLOR_RED;
}

const lv_color_t BatteryIcon::GetDefaultBatteryColor(uint8_t batteryPercent) {
  if (batteryPercent == 100) {
    return LV_COLOR_GREEN;
  }
  return LV_COLOR_WHITE;
}

const char* BatteryIcon::GetUnknownIcon() {
  return Symbols::batteryEmpty;
}

const char* BatteryIcon::GetPlugIcon(bool isCharging) {
  if (isCharging) {
    return Symbols::plug;
  }
  return "";
}
