#include "NotificationIcon.h"
#include "Symbols.h"
using namespace Pinetime::Applications::Screens;

const char* NotificationIcon::GetIcon(bool newNotificationAvailable) {
  if(newNotificationAvailable) return Symbols::info;
  else return "";
}