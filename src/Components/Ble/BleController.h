#pragma once

#include <FreeRTOS.h>>
#include <queue.h>

namespace Pinetime {
  namespace Controllers {
    class Ble {
      public:

        Ble() = default;
        bool IsConnected() const {return isConnected;}
        void Connect();
        void Disconnect();
      private:
        bool isConnected = false;

    };
  }
}