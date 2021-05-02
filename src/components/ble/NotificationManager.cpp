#include "NotificationManager.h"
#include <cstring>
#include <algorithm>

using namespace Pinetime::Controllers;

constexpr uint8_t NotificationManager::MessageSize;

void NotificationManager::Pop(Notification::Id id) {
  // notifications is a circular buffer, the *valid* items in it are contiguous
  // and sorted by id.
  // Ex:
  // 1, 2, 3, X, X (where X means invalid)
  // 6, 2, 3, 4, 5 (oldest item got overwritten)
  //
  // When dismissing a notification, every valid item to the right (wrapping)
  // that has a larger ID should be shifted to the left once
  // and the newly-created "gap" is marked invalid
  //
  // 6, 2, 3, 4, 5 (dismissing 4)
  // X, 2, 3, 5, 6

  auto to_dismiss = std::find_if(notifications.begin(), notifications.end(), [id](const Notification& n) {
    return n.valid && n.id == id;
  });
  auto idx = to_dismiss - notifications.begin();
  auto next = (idx + 1) % TotalNbNotifications;
  notifications[idx].valid = false;

  while (notifications[next].valid and notifications[next].id > id) {
    notifications[idx] = notifications[next];
    notifications[idx].index--;
    notifications[next].valid = false; // just moved this
    idx = (idx + 1) % TotalNbNotifications;
    next = (idx + 1) % TotalNbNotifications;
  }

  readIndex = readIndex == 0 ? TotalNbNotifications : readIndex - 1;
  writeIndex = writeIndex == 0 ? TotalNbNotifications : writeIndex - 1;

  // popped last notification
  if (NbNotifications() == 0) {
    empty = true;
    readIndex = 0;
    writeIndex = 0;
    ClearNewNotificationFlag();
    return;
  }
}

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

bool NotificationManager::IsVibrationEnabled() {
  return vibrationEnabled;
}

void NotificationManager::ToggleVibrations() {
  vibrationEnabled = !vibrationEnabled;
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
