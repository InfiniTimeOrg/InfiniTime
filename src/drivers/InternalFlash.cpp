#include "drivers/InternalFlash.h"
#include <mdk/nrf.h>
using namespace Pinetime::Drivers;

void InternalFlash::ErasePage(uint32_t address) {
  // Enable erase.
  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een;
  __ISB();
  __DSB();

  // Erase the page
  NRF_NVMC->ERASEPAGE = address;
  Wait();

  // Disable erase
  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
  __ISB();
  __DSB();
}

void InternalFlash::WriteWord(uint32_t address, uint32_t value) {
  // Enable write.
  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
  __ISB();
  __DSB();

  // Write word
  *(uint32_t*) address = value;
  Wait();

  // Disable write
  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
  __ISB();
  __DSB();
}

void InternalFlash::Wait() {
  while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
    ;
  }
}
