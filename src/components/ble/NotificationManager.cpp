#include "components/ble/NotificationManager.h"
#include <cstring>
#include <algorithm>

using namespace Pinetime::Controllers;

constexpr uint8_t NotificationManager::MessageSize;

void NotificationManager::Push(NotificationManager::Notification&& notif) {
  notif.id = GetNextId();
  notif.valid = true;
  notif.index = writeIndex;
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
    return {};

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
    return {};

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

void NotificationManager::Dismiss(NotificationManager::Notification::Id id) {
  if (empty)
    return;

  auto currentIterator = std::find_if(notifications.begin(), notifications.end(), [id](const Notification& n) {
    return n.valid && n.id == id;
  });
  if (currentIterator == notifications.end() || currentIterator->id != id)
    return;

  size_t count = NbNotifications();
  size_t foundIndex = std::distance(notifications.begin(), currentIterator);
  size_t foundId = notifications[foundIndex].id;

  for (size_t i = foundIndex; i < TotalNbNotifications - 1; i++) {
    notifications[i] = notifications[i + 1];
    notifications[i].index = i;
  }
  notifications[TotalNbNotifications - 1] = {};

  if (count == 1) {
    readIndex = 0;
    writeIndex = 0;
    empty = true;
  } else {
    // wrap backwards as notifications[] is a ring buffer
    writeIndex = (writeIndex > 0) ? writeIndex - 1 : TotalNbNotifications - 1;
    readIndex = (readIndex > 0) ? readIndex - 1 : TotalNbNotifications - 1;
  }
  for (size_t i = 0; i < TotalNbNotifications - 1; i++) {
    if(notifications[i].id > foundId)
      notifications[i].id--;
  }

  // Ensure any new id's are incrementing from the last notification id
  nextId = notifications[readIndex].id + 1;
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
