#include "BleIcon.h"
#include "Symbols.h"
using namespace Pinetime::Applications::Screens;

const char* BleIcon::GetIcon(bool isConnected) {
  if(isConnected) return Symbols::bluetooth;
  else return "";
}