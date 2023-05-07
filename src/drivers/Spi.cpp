#include "drivers/Spi.h"
#include <hal/nrf_gpio.h>
#include <nrfx_log.h>

using namespace Pinetime::Drivers;

Spi::Spi(SpiMaster& spiMaster, uint8_t pinCsn) : spiMaster {spiMaster}, pinCsn {pinCsn} {
  nrf_gpio_cfg_output(pinCsn);
  nrf_gpio_pin_set(pinCsn);
}

bool Spi::Write(const uint8_t* data, size_t size) {
  return spiMaster.Write(pinCsn, data, size);
}

bool Spi::Read(uint8_t* cmd, size_t cmdSize, uint8_t* data, size_t dataSize) {
  return spiMaster.Read(pinCsn, cmd, cmdSize, data, dataSize);
}

void Spi::Sleep() {
  nrf_gpio_cfg_default(pinCsn);
  NRF_LOG_INFO("[SPI] Sleep")
}

bool Spi::WriteCmdAndBuffer(const uint8_t* cmd, size_t cmdSize, const uint8_t* data, size_t dataSize) {
  return spiMaster.WriteCmdAndBuffer(pinCsn, cmd, cmdSize, data, dataSize);
}

bool Spi::Init() {
  nrf_gpio_cfg_output(pinCsn);
  nrf_gpio_pin_set(pinCsn);
  return true;
}

void Spi::Wakeup() {
  nrf_gpio_cfg_output(pinCsn);
  nrf_gpio_pin_set(pinCsn);
  NRF_LOG_INFO("[SPI] Wakeup")
}
