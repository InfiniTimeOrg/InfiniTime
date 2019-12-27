#pragma once

namespace Pinetime {
  namespace Controllers {
    class Ble {
      public:
        bool IsConnected() const {return isConnected;}
        void Connect();
        void Disconnect();

      private:
        bool isConnected = false;
    };
  }
}