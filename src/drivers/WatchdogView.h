#pragma once
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    class WatchdogView {
    public:
      explicit WatchdogView(const Watchdog& watchdog) : watchdog {watchdog} {
      }

      Watchdogs::ResetReasons ResetReason() const {
        return watchdog.ResetReason();
      }

    private:
      const Watchdog& watchdog;
    };
  }
}

