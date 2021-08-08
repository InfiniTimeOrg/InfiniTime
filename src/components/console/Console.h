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

        void Print(char *str);

      private:
          Pinetime::System::SystemTask& systemTask;
          Pinetime::Controllers::NimbleController& nimbleController;

      };
  }
}
