#include "components/ble/NotificationManager.h"
#include <cstring>
#include <algorithm>
#include <cassert>

using namespace Pinetime::Controllers;

constexpr uint8_t NotificationManager::MessageSize;

void NotificationManager::Push(NotificationManager::Notification&& notif) {
  notif.id = GetNextId();
  notif.valid = true;
  newNotification = true;
  if (beginIdx > 0) {
    --beginIdx;
  } else {
    beginIdx = notifications.size() - 1;
  }
  notifications[beginIdx] = std::move(notif);
  if (size < notifications.size()) {
    size++;
  }
}

NotificationManager::Notification::Id NotificationManager::GetNextId() {
  return nextId++;
}

NotificationManager::Notification NotificationManager::GetLastNotification() const {
  if (this->IsEmpty()) {
    return {};
  }
  return this->At(0);
}

const NotificationManager::Notification& NotificationManager::At(NotificationManager::Notification::Idx idx) const {
  if (idx >= notifications.size()) {
    assert(false);
    return notifications.at(beginIdx); // this should not happen
  }
  size_t read_idx = (beginIdx + idx) % notifications.size();
  return notifications.at(read_idx);
}

NotificationManager::Notification& NotificationManager::At(NotificationManager::Notification::Idx idx) {
  if (idx >= notifications.size()) {
    assert(false);
    return notifications.at(beginIdx); // this should not happen
  }
  size_t read_idx = (beginIdx + idx) % notifications.size();
  return notifications.at(read_idx);
}

NotificationManager::Notification::Idx NotificationManager::IndexOf(NotificationManager::Notification::Id id) const {
  for (NotificationManager::Notification::Idx idx = 0; idx < this->size; idx++) {
    const NotificationManager::Notification& notification = this->At(idx);
    if (notification.id == id) {
      return idx;
    }
  }
  return size;
}

NotificationManager::Notification NotificationManager::Get(NotificationManager::Notification::Id id) const {
  NotificationManager::Notification::Idx idx = this->IndexOf(id);
  if (idx == this->size) {
    return {};
  }
  return this->At(idx);
}

NotificationManager::Notification NotificationManager::GetNext(NotificationManager::Notification::Id id) const {
  NotificationManager::Notification::Idx idx = this->IndexOf(id);
  if (idx == this->size) {
    return {};
  }
  if (idx == 0 || idx > notifications.size()) {
    return {};
  }
  return this->At(idx - 1);
}

NotificationManager::Notification NotificationManager::GetPrevious(NotificationManager::Notification::Id id) const {
  NotificationManager::Notification::Idx idx = this->IndexOf(id);
  if (idx == this->size) {
    return {};
  }
  if (static_cast<size_t>(idx + 1) >= notifications.size()) {
    return {};
  }
  return this->At(idx + 1);
}

void NotificationManager::DismissIdx(NotificationManager::Notification::Idx idx) {
  if (this->IsEmpty()) {
    return;
  }
  if (idx >= size) {
    assert(false);
    return; // this should not happen
  }
  if (idx == 0) { // just remove the first element, don't need to change the other elements
    notifications.at(beginIdx).valid = false;
    beginIdx = (beginIdx + 1) % notifications.size();
  } else {
    // overwrite the specified entry by moving all later messages one index to the front
    for (size_t i = idx; i < size - 1; ++i) {
      this->At(i) = this->At(i + 1);
    }
    this->At(size - 1).valid = false;
  }
  --size;
}

void NotificationManager::Dismiss(NotificationManager::Notification::Id id) {
  NotificationManager::Notification::Idx idx = this->IndexOf(id);
  if (idx == this->size) {
    return;
  }
  this->DismissIdx(idx);
}

bool NotificationManager::AreNewNotificationsAvailable() const {
  return newNotification;
}

bool NotificationManager::ClearNewNotificationFlag() {
  return newNotification.exchange(false);
}

size_t NotificationManager::NbNotifications() const {
  return size;
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
