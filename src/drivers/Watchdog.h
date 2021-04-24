#pragma once
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    class Watchdog {
    public:
      enum class ResetReasons { ResetPin, Watchdog, SoftReset, CpuLockup, SystemOff, LpComp, DebugInterface, NFC, HardReset };
      void Setup(uint8_t timeoutSeconds);
      void Start();
      void Kick();
      ResetReasons ResetReason() const {
        return resetReason;
      }
      static const char* ResetReasonToString(ResetReasons reason);

    private:
      ResetReasons resetReason;
      ResetReasons ActualResetReason() const;
    };

    class WatchdogView {
    public:
      WatchdogView(const Watchdog& watchdog) : watchdog {watchdog} {
      }
      Watchdog::ResetReasons ResetReason() const {
        return watchdog.ResetReason();
      }

    private:
      const Watchdog& watchdog;
    };
  }
}
