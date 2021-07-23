#include <cstdint>
#include "BatteryIcon.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

const char* BatteryIcon::GetBatteryIcon(uint8_t batteryPercent) {
  if (batteryPercent > 90)
    return Symbols::batteryFull;
  if (batteryPercent > 75)
    return Symbols::batteryThreeQuarter;
  if (batteryPercent > 50)
    return Symbols::batteryHalf;
  if (batteryPercent > 25)
    return Symbols::batteryOneQuarter;
  return Symbols::batteryEmpty;
}

const char* BatteryIcon::GetUnknownIcon() {
  return Symbols::batteryEmpty;
}

const char* BatteryIcon::GetPlugIcon(bool isCharging) {
  if (isCharging)
    return Symbols::plug;
  else
    return "";
}
