#include <cstring>
#include <algorithm>
#include "NotificationManager.h"

using namespace Pinetime::Controllers;

constexpr uint8_t NotificationManager::MessageSize;


void NotificationManager::Push(Pinetime::Controllers::NotificationManager::Categories category,
                                                      const char *message, uint8_t currentMessageSize) {
  // TODO handle edge cases on read/write index
  auto checkedSize = std::min(currentMessageSize, MessageSize);
  auto& notif = notifications[writeIndex];
  std::memcpy(notif.message.data(), message, checkedSize);
  notif.message[checkedSize] = '\0';
  notif.category = category;
  notif.id = GetNextId();
  notif.valid = true;

  writeIndex = (writeIndex + 1 < TotalNbNotifications) ? writeIndex + 1 : 0;
  if(!empty)
    readIndex = (readIndex + 1 < TotalNbNotifications) ? readIndex + 1 : 0;
  else empty = false;

  newNotification = true;
}

NotificationManager::Notification NotificationManager::GetLastNotification() {
  NotificationManager::Notification notification = notifications[readIndex];
  return notification;
}

NotificationManager::Notification::Id NotificationManager::GetNextId() {
  return nextId++;
}

NotificationManager::Notification NotificationManager::GetNext(NotificationManager::Notification::Id id) {
  auto currentIterator = std::find_if(notifications.begin(), notifications.end(), [id](const Notification& n){return n.valid && n.id == id;});
  if(currentIterator == notifications.end() || currentIterator->id != id) return Notification{};

  auto& lastNotification = notifications[readIndex];

  NotificationManager::Notification result;

  if(currentIterator == (notifications.end()-1))
    result = *(notifications.begin());
  else
    result = *(currentIterator+1);

  if(result.id <= id) return {};

  result.index = (lastNotification.id - result.id)+1;
  return result;
}

NotificationManager::Notification NotificationManager::GetPrevious(NotificationManager::Notification::Id id) {
  auto currentIterator = std::find_if(notifications.begin(), notifications.end(), [id](const Notification& n){return n.valid && n.id == id;});
  if(currentIterator == notifications.end() || currentIterator->id != id) return Notification{};

  auto& lastNotification = notifications[readIndex];

  NotificationManager::Notification result;

  if(currentIterator == notifications.begin())
    result = *(notifications.end()-1);
  else
    result = *(currentIterator-1);

  if(result.id >= id) return {};

  result.index = (lastNotification.id - result.id)+1;
  return result;
}

bool NotificationManager::AreNewNotificationsAvailable() {
  return newNotification;
}

bool NotificationManager::ClearNewNotificationFlag() {
  return newNotification.exchange(false);
}

