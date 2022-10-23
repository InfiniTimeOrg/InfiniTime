#pragma once
#include "drivers/Spi.h"
#include "drivers/SpiMaster.h"
#include <cstdint>
#include <drivers/SpiNorFlash.h>

#ifdef TARGET_DEVICE_PINETIME
#include <drivers/nrf52/Spi.h>
#include <drivers/nrf52/SpiMaster.h>
#include <drivers/spiFlash/SpiNorFlash.h>
#endif

// #error "Do not include this"
namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
  using SpiMaster = Interface::SpiMaster<Pinetime::Drivers::Nrf52::SpiMaster>;
  using Spi = Interface::Spi<Pinetime::Drivers::Nrf52::Spi>;
  using SpiNorFlash = Interface::SpiNorFlash<Pinetime::Drivers::SpiFlash::SpiNorFlash>;
#else
  #error "No target device specified!"
#endif
  }
}