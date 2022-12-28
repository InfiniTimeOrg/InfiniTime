#pragma once
#include "drivers/Watchdog.h"
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    namespace Nrf52 {
      class Watchdog {
      public:
        enum class ResetReasons { ResetPin, Watchdog, SoftReset, CpuLockup, SystemOff, LpComp, DebugInterface, NFC, HardReset };
        void Setup(uint8_t timeoutSeconds);
        void Start();
        void Kick();
        Watchdogs::ResetReasons ResetReason() const {
          return resetReason;
        }

      private:
        Watchdogs::ResetReasons resetReason;
        Watchdogs::ResetReasons ActualResetReason() const;
      };
    }
  }
}
