#include "displayapp/screens/AlarmIcon.h"
#include "displayapp/screens/Symbols.h"
using namespace Pinetime::Applications::Screens;

const char* AlarmIcon::GetIcon(bool isSet) {
  if (isSet) {
   return Symbols::bird;
  }

  return Symbols::zzz;
}
