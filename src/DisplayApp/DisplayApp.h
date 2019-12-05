#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <drivers/st7789.h>

namespace Pinetime {
  namespace Applications {
    class DisplayApp {
      public:
        void Start();
      private:
        TaskHandle_t taskHandle;
        static void Process(void* instance);
        static void gfx_initialization();

    };
  }
}


