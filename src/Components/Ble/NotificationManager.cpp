#include <cstring>
#include <stdint.h>
#include "NotificationManager.h"

using namespace Pinetime::Controllers;

void NotificationManager::Push(Pinetime::Controllers::NotificationManager::Categories category,
                                                      const char *message, uint8_t currentMessageSize) {
  // TODO handle edge cases on read/write index
  auto checkedSize = std::min(currentMessageSize, uint8_t{18});
  auto& notif = notifications[writeIndex];
  std::memcpy(notif.message.data(), message, checkedSize);
  notif.message[checkedSize] = '\0';
  notif.category = category;

  writeIndex = (writeIndex + 1 < TotalNbNotifications) ? writeIndex + 1 : 0;
  if(!empty && writeIndex == readIndex)
    readIndex = writeIndex + 1;
}

NotificationManager::Notification Pinetime::Controllers::NotificationManager::Pop() {
// TODO handle edge cases on read/write index
  NotificationManager::Notification notification = notifications[readIndex];

  if(readIndex != writeIndex) {
    readIndex = (readIndex + 1 < TotalNbNotifications) ? readIndex + 1 : 0;
  }

  // TODO Check move optimization on return
  return notification;
}
