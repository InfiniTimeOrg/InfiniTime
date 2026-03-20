#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Components {
    class RebootPersist {
    public:
      [[nodiscard]] bool IsValid() const volatile {
        return canary == magic;
      }

      void SetValid() volatile {
        canary = magic;
      }

      // layout of time point is implementation defined
      // instead store milliseconds since epoch
      uint64_t timeMillis;
      uint32_t steps;
      uint32_t tripSteps;

    private:
      // The canary determines whether memory has been kept or not after a reset,
      // since the NRF52832 doesn't guarantee RAM retention
      // See https://docs.nordicsemi.com/bundle/ps_nrf52832/page/power.html#d935e523
      // If the magic value is still in the canary when booting,
      // we assume that memory is intact from the previous boot

      // Increment magic upon changing members of this class
      // Otherwise garbage values will be loaded after DFU
      static constexpr uint32_t magic = 0xDEAD0001;
      uint32_t canary;
    };
  }
}
