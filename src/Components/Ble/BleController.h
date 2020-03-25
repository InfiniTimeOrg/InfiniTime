#pragma once

#include <FreeRTOS.h>>
#include <queue.h>

namespace Pinetime {
  namespace Controllers {
    class Ble {
      public:
        struct NotificationMessage {
          uint8_t size = 0;
          const char* message = nullptr;
        };

        Ble();
        bool IsConnected() const {return isConnected;}
        void Connect();
        void Disconnect();

        void PushNotification(const char* message, uint8_t size);
        bool PopNotification(NotificationMessage& msg);

      private:
        bool isConnected = false;
        QueueHandle_t notificationQueue;

    };
  }
}