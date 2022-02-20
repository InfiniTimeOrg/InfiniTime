#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/Symbols.h"
using namespace Pinetime::Applications::Screens;

const char* BleIcon::GetIcon(bool isRadioEnabled, bool isConnected) {
  if(!isRadioEnabled) {
    return Symbols::airplane;
  }

  if (isConnected) {
    return Symbols::bluetooth;
  }

  return Symbols::none;
}
