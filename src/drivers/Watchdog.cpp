#include <mdk/nrf52.h>
#include <mdk/nrf52_bitfields.h>
#include <nrf_soc.h>
#include "Watchdog.h"
using namespace Pinetime::Drivers;


void Watchdog::Setup(uint8_t timeoutSeconds) {
  NRF_WDT->CONFIG &= ~(WDT_CONFIG_SLEEP_Msk << WDT_CONFIG_SLEEP_Pos);
  NRF_WDT->CONFIG |= (WDT_CONFIG_HALT_Run << WDT_CONFIG_SLEEP_Pos);

  NRF_WDT->CONFIG &= ~(WDT_CONFIG_HALT_Msk << WDT_CONFIG_HALT_Pos);
  NRF_WDT->CONFIG |= (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos);

  /* timeout (s) = (CRV + 1) / 32768 */
  // JF : 7500 = 7.5s
  uint32_t crv = (((timeoutSeconds*1000u) << 15u) / 1000) - 1;
  NRF_WDT->CRV = crv;

  /* Enable reload requests */
  NRF_WDT->RREN = (WDT_RREN_RR0_Enabled << WDT_RREN_RR0_Pos);
}

void Watchdog::Start() {
  NRF_WDT->TASKS_START = 1;
}

void Watchdog::Kick() {
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}

Watchdog::ResetReasons Watchdog::ResetReason() {
  uint32_t resetReason;
  sd_power_reset_reason_get(&resetReason);
  sd_power_reset_reason_clr(0xFFFFFFFF);
  if(resetReason & 0x01) return ResetReasons::ResetPin;
  if((resetReason >> 1) & 0x01) return ResetReasons::Watchdog;
  if((resetReason >> 2) & 0x01) return ResetReasons::SoftReset;
  if((resetReason >> 3) & 0x01) return ResetReasons::CpuLockup;
  if((resetReason >> 16) & 0x01) return ResetReasons::SystemOff;
  if((resetReason >> 17) & 0x01) return ResetReasons::LpComp;
  if((resetReason >> 18) & 0x01) return ResetReasons::DebugInterface;
  if((resetReason >> 19) & 0x01) return ResetReasons::NFC;
  return ResetReasons::HardReset;
}

const char *Watchdog::ResetReasonToString(Watchdog::ResetReasons reason) {
  switch(reason) {
    case ResetReasons::ResetPin: return "Reset pin";
    case ResetReasons::Watchdog: return "Watchdog";
    case ResetReasons::DebugInterface: return "Debug interface";
    case ResetReasons::LpComp: return "LPCOMP";
    case ResetReasons::SystemOff: return "System OFF";
    case ResetReasons::CpuLockup: return "CPU Lock-up";
    case ResetReasons::SoftReset: return "Soft reset";
    case ResetReasons::NFC: return "NFC";
    case ResetReasons::HardReset: return "Hard reset";
    default: return "Unknown";
  }
}
