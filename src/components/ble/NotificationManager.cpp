#include "components/ble/NotificationManager.h"
#include <cstring>
#include <algorithm>

using namespace Pinetime::Controllers;

constexpr uint8_t NotificationManager::MessageSize;

void NotificationManager::Push(NotificationManager::Notification&& notif) {
  notif.id = GetNextId();
  notif.valid = true;
  newNotification = true;
  Utility::ForcePush(notifications, notif);
}

NotificationManager::Notification::Id NotificationManager::GetNextId() {
  return nextId++;
}

NotificationManager::Notification NotificationManager::GetNewestNotification() const {
  if (IsEmpty()) {
    return {};
  }
  return notifications[notifications.Size() - 1];
}

NotificationManager::Notification::Idx NotificationManager::NotificationPosition(NotificationManager::Notification::Id id) const {
  Notification::Idx idx = 0;
  for (; idx < notifications.Size() && notifications[idx].id != id; idx++) {
  }
  return notifications.Size() - 1 - idx;
}

NotificationManager::Notification::Idx NotificationManager::IndexOf(NotificationManager::Notification::Id id) const {
  for (Notification::Idx idx = 0; idx < notifications.Size(); idx++) {
    if (notifications[idx].id == id) {
      return idx;
    }
  }
  return notifications.Size();
}

NotificationManager::Notification NotificationManager::Get(NotificationManager::Notification::Id id) const {
  NotificationManager::Notification::Idx idx = this->IndexOf(id);
  if (idx == notifications.Size()) {
    return {};
  }
  return notifications[idx];
}

NotificationManager::Notification NotificationManager::GetNext(NotificationManager::Notification::Id id) const {
  NotificationManager::Notification::Idx idx = this->IndexOf(id);
  if (idx >= notifications.Size() - 1) {
    return {};
  }
  return notifications[idx + 1];
}

NotificationManager::Notification NotificationManager::GetPrevious(NotificationManager::Notification::Id id) const {
  NotificationManager::Notification::Idx idx = this->IndexOf(id);
  if (idx <= 0) {
    return {};
  }
  return notifications[idx - 1];
}

void NotificationManager::DismissIdx(NotificationManager::Notification::Idx idx) {
  if (idx >= notifications.Size()) {
    return;
  }
  notifications.Erase(idx);
}

void NotificationManager::Dismiss(NotificationManager::Notification::Id id) {
  NotificationManager::Notification::Idx idx = this->IndexOf(id);
  if (idx >= notifications.Size()) {
    return;
  }
  DismissIdx(idx);
}

bool NotificationManager::AreNewNotificationsAvailable() const {
  return newNotification;
}

bool NotificationManager::ClearNewNotificationFlag() {
  return newNotification.exchange(false);
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
