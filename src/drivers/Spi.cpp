#include <hal/nrf_gpio.h>
#include "Spi.h"

using namespace Pinetime::Drivers;

Spi::Spi(SpiMaster& spiMaster, uint8_t pinCsn) :
        spiMaster{spiMaster}, pinCsn{pinCsn} {
  nrf_gpio_cfg_output(pinCsn);
  nrf_gpio_pin_set(pinCsn);
}

bool Spi::Write(const uint8_t *data, size_t size) {
  return spiMaster.Write(pinCsn, data, size);
}

bool Spi::Read(uint8_t* cmd, size_t cmdSize, uint8_t *data, size_t dataSize) {
  return spiMaster.Read(pinCsn, cmd, cmdSize, data, dataSize);
}

void Spi::Sleep() {
  // TODO sleep spi
  nrf_gpio_cfg_default(pinCsn);
}

bool Spi::Init() {
  nrf_gpio_pin_set(pinCsn); /* disable Set slave select (inactive high) */
  return true;
}

bool Spi::WriteCmdAndBuffer(uint8_t *cmd, size_t cmdSize, uint8_t *data, size_t dataSize) {
  return spiMaster.WriteCmdAndBuffer(pinCsn, cmd, cmdSize, data, dataSize);
}


