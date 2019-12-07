#include <hal/nrf_gpio.h>
#include "SpiMaster.h"

using namespace Pinetime::Drivers;

bool SpiMaster::Init(const SpiMaster::SpiModule spi, const SpiMaster::Parameters &params) {
  /* Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI0 */
  nrf_gpio_cfg_output(params.pinSCK);
  nrf_gpio_cfg_output(params.pinMOSI);
  nrf_gpio_cfg_input(params.pinMISO, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_output(params.pinCSN);
  pinCsn = params.pinCSN;

  switch(spi) {
    case SpiModule::SPI0: spiBaseAddress = NRF_SPI0; break;
    case SpiModule::SPI1: spiBaseAddress = NRF_SPI1; break;
    default: return false;
  }

  /* Configure pins, frequency and mode */
  spiBaseAddress->PSELSCK  = params.pinSCK;
  spiBaseAddress->PSELMOSI = params.pinMOSI;
  spiBaseAddress->PSELMISO = params.pinMISO;
  nrf_gpio_pin_set(pinCsn); /* disable Set slave select (inactive high) */

  uint32_t frequency;
  switch(params.Frequency) {
    case Frequencies::Freq8Mhz: frequency = 0x80000000; break;
    default: return false;
  }
  spiBaseAddress->FREQUENCY = frequency;

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

  spiBaseAddress->CONFIG = regConfig;
  spiBaseAddress->EVENTS_READY = 0;
  spiBaseAddress->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

  return true;
}

bool SpiMaster::Write(const uint8_t *data, size_t size) {
  volatile uint32_t dummyread;

  if(data == nullptr) return false;

  /* enable slave (slave select active low) */
  nrf_gpio_pin_clear(pinCsn);

  spiBaseAddress->EVENTS_READY = 0;

  spiBaseAddress->TXD = (uint32_t)*data++;

  while(--size)
  {
    spiBaseAddress->TXD =  (uint32_t)*data++;

    /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
    while (spiBaseAddress->EVENTS_READY == 0);

    /* clear the event to be ready to receive next messages */
    spiBaseAddress->EVENTS_READY = 0;

    dummyread = spiBaseAddress->RXD;
  }

  /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
  while (spiBaseAddress->EVENTS_READY == 0);

  dummyread = spiBaseAddress->RXD;

  /* disable slave (slave select active low) */
  nrf_gpio_pin_set(pinCsn);

  return true;
}
