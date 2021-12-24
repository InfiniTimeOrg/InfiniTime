#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/Symbols.h"
using namespace Pinetime::Applications::Screens;

const char* BleIcon::GetIcon(Pinetime::Controllers::Ble::ConnectStates state) {
  if (state == Pinetime::Controllers::Ble::ConnectStates::Connected)
    return Symbols::bluetooth;
  else if (state == Pinetime::Controllers::Ble::ConnectStates::Airplane)
    return Symbols::airplane;
  else
    return Symbols::none;
}
