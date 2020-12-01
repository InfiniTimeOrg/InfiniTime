#include "BatteryIcon.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

const char* BatteryIcon::GetBatteryIcon(float batteryPercent) {
  if(batteryPercent > 90.0f) return Symbols::batteryFull;
  if(batteryPercent > 75.0f) return Symbols::batteryThreeQuarter;
  if(batteryPercent > 50.0f) return Symbols::batteryHalf;
  if(batteryPercent > 25.0f) return Symbols::batteryOneQuarter;
  return Symbols::batteryEmpty;
}

const char* BatteryIcon::GetUnknownIcon() {
  return Symbols::batteryEmpty;
}

const char *BatteryIcon::GetPlugIcon(bool isCharging) {
  if(isCharging)
    return Symbols::plug;
  else return "";
}
