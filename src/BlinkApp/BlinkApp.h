#pragma once
#include <FreeRTOS.h>
#include <task.h>

namespace Pinetime {
  namespace Applications {
    class BlinkApp {
      public:
        void Start();
      private:
        TaskHandle_t taskHandle;
        static void Process(void* instance);
    };
  }
}
