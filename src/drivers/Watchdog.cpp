#include "drivers/Watchdog.h"
#include <mdk/nrf.h>
using namespace Pinetime::Drivers;

void Watchdog::Setup(uint8_t timeoutSeconds) {
  NRF_WDT->CONFIG &= ~(WDT_CONFIG_SLEEP_Msk << WDT_CONFIG_SLEEP_Pos);
  NRF_WDT->CONFIG |= (WDT_CONFIG_HALT_Run << WDT_CONFIG_SLEEP_Pos);

  NRF_WDT->CONFIG &= ~(WDT_CONFIG_HALT_Msk << WDT_CONFIG_HALT_Pos);
  NRF_WDT->CONFIG |= (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos);

  /* timeout (s) = (CRV + 1) / 32768 */
  // JF : 7500 = 7.5s
  uint32_t crv = (((timeoutSeconds * 1000u) << 15u) / 1000) - 1;
  NRF_WDT->CRV = crv;

  /* Enable reload requests */
  NRF_WDT->RREN = (WDT_RREN_RR0_Enabled << WDT_RREN_RR0_Pos);

  resetReason = ActualResetReason();
}

void Watchdog::Start() {
  NRF_WDT->TASKS_START = 1;
}

void Watchdog::Kick() {
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}

Watchdog::ResetReasons Watchdog::ActualResetReason() const {
  uint32_t reason = NRF_POWER->RESETREAS;
  NRF_POWER->RESETREAS = 0xffffffff;

  if (reason & 0x01u)
    return ResetReasons::ResetPin;
  if ((reason >> 1u) & 0x01u)
    return ResetReasons::Watchdog;
  if ((reason >> 2u) & 0x01u)
    return ResetReasons::SoftReset;
  if ((reason >> 3u) & 0x01u)
    return ResetReasons::CpuLockup;
  if ((reason >> 16u) & 0x01u)
    return ResetReasons::SystemOff;
  if ((reason >> 17u) & 0x01u)
    return ResetReasons::LpComp;
  if ((reason) &0x01u)
    return ResetReasons::DebugInterface;
  if ((reason >> 19u) & 0x01u)
    return ResetReasons::NFC;
  return ResetReasons::HardReset;
}

const char* Watchdog::ResetReasonToString(Watchdog::ResetReasons reason) {
  switch (reason) {
    case ResetReasons::ResetPin:
      return "Reset pin";
    case ResetReasons::Watchdog:
      return "Watchdog";
    case ResetReasons::DebugInterface:
      return "Debug interface";
    case ResetReasons::LpComp:
      return "LPCOMP";
    case ResetReasons::SystemOff:
      return "System OFF";
    case ResetReasons::CpuLockup:
      return "CPU Lock-up";
    case ResetReasons::SoftReset:
      return "Soft reset";
    case ResetReasons::NFC:
      return "NFC";
    case ResetReasons::HardReset:
      return "Hard reset";
    default:
      return "Unknown";
  }
}
