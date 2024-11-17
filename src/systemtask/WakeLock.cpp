#include "systemtask/WakeLock.h"

using namespace Pinetime::System;

WakeLock::WakeLock(SystemTask& systemTask) : systemTask {systemTask} {
  lockHeld = false;
}

WakeLock::~WakeLock() {
  Release();
}

void WakeLock::Lock() {
  if (lockHeld) {
    return;
  }
  systemTask.PushMessage(Messages::DisableSleeping);
  lockHeld = true;
}

void WakeLock::Release() {
  if (!lockHeld) {
    return;
  }
  systemTask.PushMessage(Messages::EnableSleeping);
  lockHeld = false;
}
