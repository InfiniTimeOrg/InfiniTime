#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
using namespace Pinetime::Applications::Screens;

const char* NotificationIcon::GetIcon(bool newNotificationAvailable) {
  if (newNotificationAvailable)
    return Symbols::info;
  else
    return "";
}

//TODO: does this really belong here?
const char* NotificationIcon::GetCategoryIcon(Pinetime::Controllers::NotificationManager::Categories category) {
  switch (category) {
    //TODO add symbols for each category
    default:
      return Symbols::info;
  }  
}