#include "NotificationManager.h"
#include <cstring>
#include <algorithm>

using namespace Pinetime::Controllers;

constexpr uint8_t NotificationManager::MessageSize;

void NotificationManager::Push(NotificationManager::Notification&& notif) {
  notif.id = GetNextId();
  notif.valid = true;
  notifications[writeIndex] = std::move(notif);
  writeIndex = (writeIndex + 1 < TotalNbNotifications) ? writeIndex + 1 : 0;
  if (!empty)
    readIndex = (readIndex + 1 < TotalNbNotifications) ? readIndex + 1 : 0;
  else
    empty = false;

  newNotification = true;
}

NotificationManager::Notification NotificationManager::GetLastNotification() {
  NotificationManager::Notification notification = notifications[readIndex];
  notification.index = 1;
  return notification;
}

NotificationManager::Notification::Id NotificationManager::GetNextId() {
  return nextId++;
}

NotificationManager::Notification NotificationManager::GetNext(NotificationManager::Notification::Id id) {
  auto currentIterator = std::find_if(notifications.begin(), notifications.end(), [id](const Notification& n) {
    return n.valid && n.id == id;
  });
  if (currentIterator == notifications.end() || currentIterator->id != id)
    return Notification {};

  auto& lastNotification = notifications[readIndex];

  NotificationManager::Notification result;

  if (currentIterator == (notifications.end() - 1))
    result = *(notifications.begin());
  else
    result = *(currentIterator + 1);

  if (result.id <= id)
    return {};

  result.index = (lastNotification.id - result.id) + 1;
  return result;
}

NotificationManager::Notification NotificationManager::GetPrevious(NotificationManager::Notification::Id id) {
  auto currentIterator = std::find_if(notifications.begin(), notifications.end(), [id](const Notification& n) {
    return n.valid && n.id == id;
  });
  if (currentIterator == notifications.end() || currentIterator->id != id)
    return Notification {};

  auto& lastNotification = notifications[readIndex];

  NotificationManager::Notification result;

  if (currentIterator == notifications.begin())
    result = *(notifications.end() - 1);
  else
    result = *(currentIterator - 1);

  if (result.id >= id)
    return {};

  result.index = (lastNotification.id - result.id) + 1;
  return result;
}

bool NotificationManager::AreNewNotificationsAvailable() {
  return newNotification;
}

bool NotificationManager::ClearNewNotificationFlag() {
  return newNotification.exchange(false);
}

size_t NotificationManager::NbNotifications() const {
  return std::count_if(notifications.begin(), notifications.end(), [](const Notification& n) {
    return n.valid;
  });
}

const char* NotificationManager::Notification::Message() const {
  const char* itField = std::find(message.begin(), message.begin() + size - 1, '\0');
  if (itField != message.begin() + size - 1) {
    const char* ptr = (itField) + 1;
    return ptr;
  }
  return const_cast<char*>(message.data());
}

const char* NotificationManager::Notification::Title() const {
  const char* itField = std::find(message.begin(), message.begin() + size - 1, '\0');
  if (itField != message.begin() + size - 1) {
    return message.data();
  }
  return {};
}
