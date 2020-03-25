#include <cstring>
#include <cstdlib>
#include "BleController.h"

using namespace Pinetime::Controllers;

Ble::Ble() {
  notificationQueue = xQueueCreate(10, sizeof(NotificationMessage));
}

void Ble::Connect() {
  isConnected = true;
}

void Ble::Disconnect() {
  isConnected = false;
}

void Ble::PushNotification(const char *message, uint8_t size) {
  char* messageCopy = static_cast<char *>(malloc(sizeof(char) * size));
  std::memcpy(messageCopy, message, size);
  NotificationMessage msg;
  msg.size = size;
  msg.message = messageCopy;

  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(notificationQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}

bool Ble::PopNotification(Ble::NotificationMessage& msg) {
  return xQueueReceive(notificationQueue, &msg, 0) != 0;
}


