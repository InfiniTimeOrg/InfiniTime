#include "drivers/Watchdog.h"
#include <mdk/nrf.h>
using namespace Pinetime::Drivers;

namespace {
  /// The watchdog is always driven by a 32768kHz clock
  constexpr uint32_t ClockFrequency = 32768;
  /// Write this value in the reload register to reload the watchdog
  constexpr uint32_t ReloadValue = 0x6E524635UL;

  /// Configures the behaviours (pause or run) of the watchdog while the CPU is sleeping or halted by the debugger
  ///
  /// @param sleepBehaviour Configure the watchdog to either be paused, or kept running, while the CPU is sleeping
  /// @param haltBehaviour Configure the watchdog to either be paused, or kept running, while the CPU is halted by the debugger
  void SetBehaviours(Watchdog::SleepBehaviour sleepBehaviour, Watchdog::HaltBehaviour haltBehaviour) {
    // NRF_WDT->CONFIG : only the 1st and 4th bits are relevant.
    // Bit 0 : Behavior when the CPU is sleeping
    // Bit 3 : Behavior when the CPU is halted by the debugger
    // O means that the CPU is paused during sleep/halt, 1 means that the watchdog is kept running
    NRF_WDT->CONFIG = static_cast<uint32_t>(sleepBehaviour) | static_cast<uint32_t>(haltBehaviour);
  }

  /// Configure the timeout delay of the watchdog (called CRV, Counter Reload Value, in the documentation).
  ///
  /// @param timeoutSeconds Timeout of the watchdog, expressed in seconds
  void SetTimeout(uint8_t timeoutSeconds) {
    // According to the documentation:
    //  Clock = 32768
    //  timeout [s] = ( CRV + 1 ) / Clock
    //  -> CRV = (timeout [s] * Clock) -1
    NRF_WDT->CRV = (timeoutSeconds * ClockFrequency) - 1;
  }

  /// Enables the first reload register
  ///
  /// The hardware provides 8 reload registers. To reload the watchdog, all enabled
  /// register must be refreshed.
  ///
  /// This driver only enables the first reload register.
  void EnableFirstReloadRegister() {
    // RRED (Reload Register Enable) is a bitfield of 8 bits. Each bit represent
    // one of the eight reload registers available.
    // In this case, we enable only the first one.
    NRF_WDT->RREN = NRF_WDT->RREN | 1;
  }

  /// Returns the reset reason provided by the POWER subsystem
  Watchdog::ResetReason GetResetReason() {
    /* NRF_POWER->RESETREAS
     * -------------------------------------------------------------------------------------------------------------------- *
     * Bit | Reason (if bit is set to 1)
     * ----|---------------------------------------------------------------------------------------------------------------- *
     *  0  | Reset from the pin reset
     *  1  | Reset from the watchdog
     *  2  | Reset from soft reset
     *  3  | Reset from CPU lock-up
     * 16  | Reset due to wake up from System OFF mode when wakeup is triggered from DETECT signal from GPIO
     * 17  | Reset due to wake up from System OFF mode when wakeup is triggered from ANADETECT signal from LPCOMP
     * 18  | Reset due to wake up from System OFF mode when wakeup is triggered from entering into debug interface mode
     * 19  | Reset due to wake up from System OFF mode by NFC field detect
     * -------------------------------------------------------------------------------------------------------------------- */
    const uint32_t reason = NRF_POWER->RESETREAS;
    NRF_POWER->RESETREAS = 0xffffffff;

    uint32_t value = reason & 0x01; // avoid implicit conversion to bool using this temporary variable.
    if (value != 0) {
      return Watchdog::ResetReason::ResetPin;
    }

    value = (reason >> 1u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::Watchdog;
    }

    value = (reason >> 2u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::SoftReset;
    }

    value = (reason >> 3u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::CpuLockup;
    }

    value = (reason >> 16u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::SystemOff;
    }

    value = (reason >> 17u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::LpComp;
    }

    value = (reason >> 18u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::DebugInterface;
    }

    value = (reason >> 19u) & 0x01u;
    if (value != 0) {
      return Watchdog::ResetReason::NFC;
    }

    return Watchdog::ResetReason::HardReset;
  }
}

void Watchdog::Setup(uint8_t timeoutSeconds, SleepBehaviour sleepBehaviour, HaltBehaviour haltBehaviour) {
  SetBehaviours(sleepBehaviour, haltBehaviour);
  SetTimeout(timeoutSeconds);
  EnableFirstReloadRegister();

  resetReason = ::GetResetReason();
}

void Watchdog::Start() {
  // Write 1 in the START task to start the watchdog
  NRF_WDT->TASKS_START = 1;
}

void Watchdog::Reload() {
  // Write the reload value 0x6E524635UL to the reload register to reload the watchdog.
  // NOTE : This driver enables only the 1st reload register.
  NRF_WDT->RR[0] = ReloadValue;
}

const char* Pinetime::Drivers::ResetReasonToString(Watchdog::ResetReason reason) {
  switch (reason) {
    case Watchdog::ResetReason::ResetPin:
      return "Reset pin";
    case Watchdog::ResetReason::Watchdog:
      return "Watchdog";
    case Watchdog::ResetReason::DebugInterface:
      return "Debug interface";
    case Watchdog::ResetReason::LpComp:
      return "LPCOMP";
    case Watchdog::ResetReason::SystemOff:
      return "System OFF";
    case Watchdog::ResetReason::CpuLockup:
      return "CPU Lock-up";
    case Watchdog::ResetReason::SoftReset:
      return "Soft reset";
    case Watchdog::ResetReason::NFC:
      return "NFC";
    case Watchdog::ResetReason::HardReset:
      return "Hard reset";
    default:
      return "Unknown";
  }
}
