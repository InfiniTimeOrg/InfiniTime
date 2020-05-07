#include <hal/nrf_gpio.h>
#include "Spi.h"

using namespace Pinetime::Drivers;

Spi::Spi(SpiMaster& spiMaster, uint8_t pinCsn) :
        spiMaster{spiMaster}, pinCsn{pinCsn} {

}

bool Spi::Write(const uint8_t *data, size_t size) {
  return spiMaster.Write(pinCsn, data, size);
}

bool Spi::Read(uint8_t *data, size_t size) {
  return spiMaster.Read(pinCsn, data, size);
}

void Spi::Sleep() {
  // TODO sleep spi
  nrf_gpio_cfg_default(pinCsn);
}

bool Spi::Init() {
  nrf_gpio_pin_set(pinCsn); /* disable Set slave select (inactive high) */
  return true;
}


