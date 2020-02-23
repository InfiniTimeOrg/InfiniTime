#pragma once

namespace Pinetime {
  namespace Drivers {
    class Watchdog {
      public:
        enum class ResetReasons { ResetPin, Watchdog, SoftReset, CpuLockup, SystemOff, LpComp, DebugInterface, NFC, HardReset };
        void Setup(uint8_t timeoutSeconds);
        void Start();
        void Kick();

        ResetReasons ResetReason();
        static const char* ResetReasonToString(ResetReasons reason);

    };
  }
}
