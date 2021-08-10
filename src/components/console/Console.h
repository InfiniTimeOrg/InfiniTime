#pragma once


#include <cstdint>
#include <cstddef>

//#include "systemtask/SystemTask.h"

namespace Pinetime {

  namespace System {
    class SystemTask;
  }

  namespace Controllers
  {
    class NimbleController;
  }

  namespace Components
  {

    class Console {
      public:
        Console(Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::NimbleController& nimbleController);

        void Init();
        void Print(char *str);
        void Received(char* str, int length);

      private:
          Pinetime::System::SystemTask& systemTask;
          Pinetime::Controllers::NimbleController& nimbleController;

          static constexpr int bufferSize = 256;
          char rxBuffer[bufferSize];
          uint16_t rxPos;

      };
  }
}
