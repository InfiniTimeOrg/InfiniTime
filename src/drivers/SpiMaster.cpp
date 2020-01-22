#include <hal/nrf_gpio.h>
#include <hal/nrf_spim.h>
#include "SpiMaster.h"
#include <algorithm>
using namespace Pinetime::Drivers;

SpiMaster* spiInstance;
SpiMaster::SpiMaster(const SpiMaster::SpiModule spi, const SpiMaster::Parameters &params) :
        spi{spi}, params{params} {
  spiInstance = this;
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

  NRF_SPIM0->INTENSET = ((unsigned)1 << (unsigned)6);
  NRF_SPIM0->INTENSET = ((unsigned)1 << (unsigned)1);
  NRF_SPIM0->INTENSET = ((unsigned)1 << (unsigned)19);

  NRF_SPIM0->ENABLE = (SPIM_ENABLE_ENABLE_Enabled << SPIM_ENABLE_ENABLE_Pos);

  NRFX_IRQ_PRIORITY_SET(SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn,6);
  NRFX_IRQ_ENABLE(SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn);
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

void SpiMaster::irqStarted() {
  if(busy) {
    auto s = currentBufferSize;
    if(s > 0) {
      auto currentSize = std::min((size_t)255, s);

      NRF_SPIM0->TXD.PTR = (uint32_t) currentBufferAddr;
      NRF_SPIM0->TXD.MAXCNT = currentSize;
      NRF_SPIM0->TXD.LIST = 0;

      currentBufferAddr += currentSize;
      currentBufferSize -= currentSize;

      NRF_SPIM0->RXD.PTR = (uint32_t) 0;
      NRF_SPIM0->RXD.MAXCNT = 0;
      NRF_SPIM0->RXD.LIST = 0;

      if(repeat == 0)
        NRF_SPIM0->SHORTS = 0;

      return;
    }else {
      if(repeat > 0) {
        repeat = repeat -1;

        currentBufferAddr = bufferAddr;
        currentBufferSize = bufferSize;
        s = currentBufferSize;
        auto currentSize = std::min((size_t)255, s);
        NRF_SPIM0->TXD.PTR = (uint32_t) currentBufferAddr;
        NRF_SPIM0->TXD.MAXCNT = currentSize;
        NRF_SPIM0->TXD.LIST = 0;

        currentBufferAddr += currentSize;
        currentBufferSize -= currentSize;

        NRF_SPIM0->RXD.PTR = (uint32_t) 0;
        NRF_SPIM0->RXD.MAXCNT = 0;
        NRF_SPIM0->RXD.LIST = 0;
      }
    }
  }
}

void SpiMaster::irqEnd() {
  if(busy) {
    if(repeat == 0 && currentBufferSize == 0) {
      nrf_gpio_pin_set(pinCsn);
      busy = false;
    }
  }
}


bool SpiMaster::Write(const uint8_t *data, size_t size, size_t r) {
  if(data == nullptr) return false;

  while(busy) {
    asm("nop");
  }

  if(size == 1) {
    setup_workaround_for_ftpan_58(NRF_SPIM0, 0,0);
    NRF_SPIM0->INTENCLR = (1<<6);
    NRF_SPIM0->INTENCLR = (1<<1);
    NRF_SPIM0->INTENCLR = (1<<19);
  } else {
    NRF_GPIOTE->CONFIG[0] = 0;
    NRF_PPI->CH[0].EEP = 0;
    NRF_PPI->CH[0].TEP = 0;
    NRF_PPI->CHENSET = 0;
    NRF_SPIM0->INTENSET = (1<<6);
    NRF_SPIM0->INTENSET = (1<<1);
    NRF_SPIM0->INTENSET = (1<<19);
  }

  nrf_gpio_pin_clear(pinCsn);

  currentBufferAddr = bufferAddr = (uint32_t)data;
  currentBufferSize = bufferSize = size;
  repeat = r;
  busy = true;

  if(repeat > 0)
    NRF_SPIM0->SHORTS = (1<<17);

  auto currentSize = std::min((size_t)255, bufferSize);
  NRF_SPIM0->TXD.PTR = bufferAddr;
  NRF_SPIM0->TXD.MAXCNT = currentSize;
  NRF_SPIM0->TXD.LIST = 0;

  currentBufferSize -= currentSize;
  currentBufferAddr += currentSize;

  NRF_SPIM0->RXD.PTR = (uint32_t) 0;
  NRF_SPIM0->RXD.MAXCNT = 0;
  NRF_SPIM0->RXD.LIST = 0;
  NRF_SPIM0->EVENTS_END = 0;
  NRF_SPIM0->TASKS_START = 1;

  if(size == 1) {
    while (NRF_SPIM0->EVENTS_END == 0);
    busy = false;
    nrf_gpio_pin_set(pinCsn);
  }


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

void SpiMaster::Wait() {
  while(busy) {
    asm("nop");
  }
}
