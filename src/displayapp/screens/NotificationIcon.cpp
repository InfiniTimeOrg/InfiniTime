#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
using namespace Pinetime::Applications::Screens;

const char* NotificationIcon::GetIcon(bool newNotificationAvailable) {
  if (newNotificationAvailable)
    return Symbols::info;
  else
    return "";
}

// TODO: does this really belong here?
const char* NotificationIcon::GetCategoryIcon(Pinetime::Controllers::NotificationManager::Categories category) {
  switch (category) {
    case Pinetime::Controllers::NotificationManager::Categories::SimpleAlert:
      return Symbols::bell;
    case Pinetime::Controllers::NotificationManager::Categories::Email:
      return Symbols::envelope;
    case Pinetime::Controllers::NotificationManager::Categories::News:
      return Symbols::newspaper;
    case Pinetime::Controllers::NotificationManager::Categories::IncomingCall:
      return Symbols::phone;
    case Pinetime::Controllers::NotificationManager::Categories::MissedCall:
      return Symbols::phoneSlash;
    case Pinetime::Controllers::NotificationManager::Categories::Sms:
      return Symbols::smsBubble;
    case Pinetime::Controllers::NotificationManager::Categories::VoiceMail:
      return Symbols::voicemail;
    case Pinetime::Controllers::NotificationManager::Categories::Schedule:
      return Symbols::clock;
    case Pinetime::Controllers::NotificationManager::Categories::HighProriotyAlert:
      return Symbols::warningTriangle;
    case Pinetime::Controllers::NotificationManager::Categories::InstantMessage:
      return Symbols::messageBubble;
    case Pinetime::Controllers::NotificationManager::Categories::Unknown:
    default:
      return Symbols::questionCircle;
  }
}