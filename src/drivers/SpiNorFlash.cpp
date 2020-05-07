#include <hal/nrf_gpio.h>
#include <libraries/delay/nrf_delay.h>
#include <libraries/log/nrf_log.h>
#include "SpiNorFlash.h"
#include "Spi.h"

using namespace Pinetime::Drivers;

SpiNorFlash::SpiNorFlash(Spi& spi) : spi{spi} {

}

void SpiNorFlash::Init() {
  uint8_t cmd = 0x9F;
  spi.Write(&cmd, 1);

  uint8_t data[3];
  data[0] = 0;
  data[1] = 0;
  data[2] = 0;
  spi.Read(data, 3);

  NRF_LOG_INFO("Manufacturer : %d, Device : %d", data[0], (data[1] + (data[2]<<8)));
}

void SpiNorFlash::Uninit() {

}

void SpiNorFlash::Sleep() {

}

void SpiNorFlash::Wakeup() {

}
