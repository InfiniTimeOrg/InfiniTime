#include "displayapp/screens/AlarmIcon.h"
#include "displayapp/screens/Symbols.h"
using namespace Pinetime::Applications::Screens;

const char* AlarmIcon::GetIcon(bool isSet) {
  if (isSet) {
<<<<<<< HEAD
    return Symbols::bell;
  }

  return Symbols::notbell;
=======
    return Symbols::bird;
  }

  return Symbols::zzz;
>>>>>>> f5dfbc44 (added symbols for alarm states, added alarm state and hour to watchface meow, changed steps to paw. Need to align everythign nicely and find where to change release number)
}
