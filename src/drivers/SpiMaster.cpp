#include <hal/nrf_gpio.h>
#include <hal/nrf_spim.h>
#include "SpiMaster.h"
#include <algorithm>
using namespace Pinetime::Drivers;

SpiMaster::SpiMaster(const SpiMaster::SpiModule spi, const SpiMaster::Parameters &params) :
        spi{spi}, params{params} {
}

bool SpiMaster::Init() {
  /* Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI0 */
  nrf_gpio_pin_set(params.pinSCK);
  nrf_gpio_cfg_output(params.pinSCK);
  nrf_gpio_pin_clear(params.pinMOSI);
  nrf_gpio_cfg_output(params.pinMOSI);
  nrf_gpio_cfg_input(params.pinMISO, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_output(params.pinCSN);
  pinCsn = params.pinCSN;

  switch(spi) {
    case SpiModule::SPI0: spiBaseAddress = NRF_SPIM0; break;
    case SpiModule::SPI1: spiBaseAddress = NRF_SPIM1; break;
    default: return false;
  }

  /* Configure pins, frequency and mode */
  NRF_SPIM0->PSELSCK  = params.pinSCK;
  NRF_SPIM0->PSELMOSI = params.pinMOSI;
  NRF_SPIM0->PSELMISO = params.pinMISO;
  nrf_gpio_pin_set(pinCsn); /* disable Set slave select (inactive high) */

  uint32_t frequency;
  switch(params.Frequency) {
    case Frequencies::Freq8Mhz: frequency = 0x80000000; break;
    default: return false;
  }
  NRF_SPIM0->FREQUENCY = frequency;

  uint32_t regConfig = 0;
  switch(params.bitOrder) {
    case BitOrder::Msb_Lsb: break;
    case BitOrder::Lsb_Msb: regConfig = 1;
    default: return false;
  }
  switch(params.mode) {
    case Modes::Mode0: break;
    case Modes::Mode1: regConfig |= (0x01 << 1); break;
    case Modes::Mode2: regConfig |= (0x02 << 1); break;
    case Modes::Mode3: regConfig |= (0x03 << 1); break;
    default: return false;
  }

  setup_workaround_for_ftpan_58(NRF_SPIM0, 0, 0);

  NRF_SPIM0->CONFIG = regConfig;
  NRF_SPIM0->EVENTS_ENDRX = 0;
  NRF_SPIM0->EVENTS_ENDTX = 0;
  NRF_SPIM0->EVENTS_END = 0;
  NRF_SPI0->EVENTS_READY = 0;
  NRF_SPI0->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);
  NRF_SPIM0->ENABLE = (SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos);

  return true;
}

bool SpiMaster::WriteFast(const uint8_t *data, size_t size) {
  auto spi = reinterpret_cast<NRF_SPI_Type*>(spiBaseAddress);
  volatile uint32_t dummyread;

  if(data == nullptr) return false;

  /* enable slave (slave select active low) */
  nrf_gpio_pin_clear(pinCsn);

  NRF_SPI0->EVENTS_READY = 0;

  NRF_SPI0->TXD = (uint32_t)*data++;

  while(--size)
  {
    NRF_SPI0->TXD =  (uint32_t)*data++;

    /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
    while (NRF_SPI0->EVENTS_READY == 0);

    /* clear the event to be ready to receive next messages */
    NRF_SPI0->EVENTS_READY = 0;

    dummyread = NRF_SPI0->RXD;
  }

  /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
  while (NRF_SPI0->EVENTS_READY == 0);

  dummyread = NRF_SPI0->RXD;

  /* disable slave (slave select active low) */
  nrf_gpio_pin_set(pinCsn);

  return true;
}

void SpiMaster::setup_workaround_for_ftpan_58(NRF_SPIM_Type *spim, uint32_t ppi_channel, uint32_t gpiote_channel) {
  // Create an event when SCK toggles.
  NRF_GPIOTE->CONFIG[gpiote_channel] = (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
                                       (spim->PSEL.SCK << GPIOTE_CONFIG_PSEL_Pos) |
                                       (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos);

  // Stop the spim instance when SCK toggles.
  NRF_PPI->CH[ppi_channel].EEP = (uint32_t) &NRF_GPIOTE->EVENTS_IN[gpiote_channel];
  NRF_PPI->CH[ppi_channel].TEP = (uint32_t) &spim->TASKS_STOP;
  NRF_PPI->CHENSET = 1U << ppi_channel;
}

bool SpiMaster::Write(const uint8_t *data, size_t size) {
  if(data == nullptr) return false;

  if(size == 1) {
    setup_workaround_for_ftpan_58(NRF_SPIM0, 0,0);
  } else {
    NRF_GPIOTE->CONFIG[0] = 0;
    NRF_PPI->CH[0].EEP = 0;
    NRF_PPI->CH[0].TEP = 0;
    NRF_PPI->CHENSET = 0;
  }

  nrf_gpio_pin_clear(pinCsn);
  auto spim = reinterpret_cast<NRF_SPIM_Type *>(spiBaseAddress);

  while(size > 0) {
    auto currentSize = std::min((size_t)255, size);
    size -= currentSize;
    NRF_SPIM0->TXD.PTR = (uint32_t) data;
    NRF_SPIM0->TXD.MAXCNT = currentSize;
    NRF_SPIM0->TXD.LIST = 0;

    NRF_SPIM0->RXD.PTR = (uint32_t) 0;
    NRF_SPIM0->RXD.MAXCNT = 0;
    NRF_SPIM0->RXD.LIST = 0;

    NRF_SPIM0->EVENTS_END = 0;

    NRF_SPIM0->TASKS_START = 1;


    while (NRF_SPIM0->EVENTS_END == 0);
  }

  nrf_gpio_pin_set(pinCsn);

  return true;
}

bool SpiMaster::GetStatusEnd() {
  return (bool)*(volatile uint32_t *)((uint8_t *)spiBaseAddress + (uint32_t)NRF_SPIM_EVENT_END);
}

bool SpiMaster::GetStatusStarted() {
  return (bool)*(volatile uint32_t *)((uint8_t *)spiBaseAddress + (uint32_t)NRF_SPIM_EVENT_STARTED);
}

void SpiMaster::Sleep() {
  while(NRF_SPIM0->ENABLE != 0) {
    NRF_SPIM0->ENABLE = (SPIM_ENABLE_ENABLE_Disabled << SPIM_ENABLE_ENABLE_Pos);
  }
  nrf_gpio_cfg_default(params.pinSCK);
  nrf_gpio_cfg_default(params.pinMOSI);
  nrf_gpio_cfg_default(params.pinMISO);
  nrf_gpio_cfg_default(params.pinCSN);
}

void SpiMaster::Wakeup() {
  Init();
}
