#pragma once
#include <cstdint>
#include <nrf52_bitfields.h>

namespace Pinetime {
  namespace Drivers {
    /// Low level driver for the watchdog based on the nRF52832 Product Specification V1.1
    ///
    /// This driver initializes the timeout and sleep and halt behaviours of the watchdog
    /// in the method Watchdog::Setup().
    ///
    /// The watchdog can then be started using the method Watchdog::Start(). At this point, the watchdog runs
    /// and will reset the MCU if it's not reloaded before the timeout elapses.
    ///
    /// The watchdog can be reloaded using Watchdog::Kick().
    ///
    /// The watchdog also provide the cause of the last reset (reset pin, watchdog, soft reset, hard reset,... See
    /// Watchdog::ResetReasons).
    class Watchdog {
    public:
      /// Indicates the reasons of a reset of the MCU
      enum class ResetReason { ResetPin, Watchdog, SoftReset, CpuLockup, SystemOff, LpComp, DebugInterface, NFC, HardReset };

      /// Behaviours of the watchdog when the CPU is sleeping
      enum class SleepBehaviour : uint8_t {
        /// Pause watchdog while the CPU is sleeping
        Pause = 0 << WDT_CONFIG_SLEEP_Pos,
        /// Keep the watchdog running while the CPU is sleeping
        Run = 1 << WDT_CONFIG_SLEEP_Pos
      };

      /// Behaviours of the watchdog when the CPU is halted by the debugger
      enum class HaltBehaviour : uint8_t {
        /// Pause watchdog while the CPU is halted by the debugger
        Pause = 0 << WDT_CONFIG_HALT_Pos,
        /// Keep the watchdog running while the CPU is halted by the debugger
        Run = 1 << WDT_CONFIG_HALT_Pos
      };

      /// Configures the watchdog with a specific timeout, behaviour when sleeping and when halted by the debugger
      ///
      /// @param sleepBehaviour Configure the watchdog to either be paused, or kept running, while the CPU is sleeping
      /// @param haltBehaviour Configure the watchdog to either be paused, or kept running, while the CPU is halted by the debugger
      void Setup(uint8_t timeoutSeconds, SleepBehaviour sleepBehaviour, HaltBehaviour haltBehaviour);

      /// Starts the watchdog. The watchdog will reset the MCU when the timeout period is elapsed unless you call
      /// Watchdog::Kick before the end of the period
      void Start();

      /// Reloads the watchdog.
      ///
      /// Ensure that you call this function regularly with a period shorter
      /// than the timeout period to prevent the watchdog from resetting the MCU.
      void Reload();

      /// Returns the reason of the last reset
      ResetReason GetResetReason() const {
        return resetReason;
      }

    private:
      ResetReason resetReason;
    };

    /// Converts a reset reason to a human readable string
    const char* ResetReasonToString(Watchdog::ResetReason reason);
  }
}
