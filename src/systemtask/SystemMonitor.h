#pragma once
#include <FreeRTOS.h> // declares configUSE_TRACE_FACILITY
#include <task.h>

namespace Pinetime {
  namespace System {
    class SystemMonitor {
    public:
      void Process();
#if configUSE_TRACE_FACILITY == 1
    private:
      mutable TickType_t lastTick = 0;
#endif
    };
  }
}
