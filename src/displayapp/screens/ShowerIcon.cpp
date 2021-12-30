#include "displayapp/screens/ShowerIcon.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

const char* ShowerIcon::GetShowerIcon(bool isEnabled) {
  if (isEnabled)
    return Symbols::shower;
  else
    return Symbols::none;
  
}