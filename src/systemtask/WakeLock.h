#pragma once

#include "systemtask/SystemTask.h"

namespace Pinetime {
  namespace System {
    class WakeLock {
    public:
      WakeLock(SystemTask& systemTask);
      ~WakeLock();
      void Lock();
      void Release();

    private:
      bool lockHeld;
      SystemTask& systemTask;
    };
  }
}
