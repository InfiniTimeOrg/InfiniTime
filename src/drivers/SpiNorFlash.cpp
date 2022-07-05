#include "drivers/SpiNorFlash.h"
#include <hal/nrf_gpio.h>
#include <libraries/delay/nrf_delay.h>
#include <libraries/log/nrf_log.h>
#include "drivers/Spi.h"

using namespace Pinetime::Drivers;

SpiNorFlash::SpiNorFlash(Spi& spi) : spi {spi} {
}

void SpiNorFlash::Init() {
  device_id = ReadIdentificaion();
  NRF_LOG_INFO("[SpiNorFlash] Manufacturer : %d, Memory type : %d, memory density : %d",
               device_id.manufacturer,
               device_id.type,
               device_id.density);
}

void SpiNorFlash::Uninit() {
}

void SpiNorFlash::Sleep() {
  auto cmd = static_cast<uint8_t>(Commands::DeepPowerDown);
  spi.Write(&cmd, sizeof(uint8_t));
  NRF_LOG_INFO("[SpiNorFlash] Sleep")
}

void SpiNorFlash::Wakeup() {
  // send Commands::ReleaseFromDeepPowerDown then 3 dummy bytes before reading Device ID
  static constexpr uint8_t cmdSize = 4;
  uint8_t cmd[cmdSize] = {static_cast<uint8_t>(Commands::ReleaseFromDeepPowerDown), 0x01, 0x02, 0x03};
  uint8_t id = 0;
  spi.Read(reinterpret_cast<uint8_t*>(&cmd), cmdSize, &id, 1);
  auto devId = device_id = ReadIdentificaion();
  if (devId.type != device_id.type) {
    NRF_LOG_INFO("[SpiNorFlash] ID on Wakeup: Failed");
  } else {
    NRF_LOG_INFO("[SpiNorFlash] ID on Wakeup: %d", id);
  }
  NRF_LOG_INFO("[SpiNorFlash] Wakeup")
}

SpiNorFlash::Identification SpiNorFlash::ReadIdentificaion() {
  auto cmd = static_cast<uint8_t>(Commands::ReadIdentification);
  Identification identification;
  spi.Read(&cmd, 1, reinterpret_cast<uint8_t*>(&identification), sizeof(Identification));
  return identification;
}

uint8_t SpiNorFlash::ReadStatusRegister() {
  auto cmd = static_cast<uint8_t>(Commands::ReadStatusRegister);
  uint8_t status;
  spi.Read(&cmd, sizeof(cmd), &status, sizeof(uint8_t));
  return status;
}

bool SpiNorFlash::WriteInProgress() {
  return (ReadStatusRegister() & 0x01u) == 0x01u;
}

bool SpiNorFlash::WriteEnabled() {
  return (ReadStatusRegister() & 0x02u) == 0x02u;
}

uint8_t SpiNorFlash::ReadConfigurationRegister() {
  auto cmd = static_cast<uint8_t>(Commands::ReadConfigurationRegister);
  uint8_t status;
  spi.Read(&cmd, sizeof(cmd), &status, sizeof(uint8_t));
  return status;
}

void SpiNorFlash::Read(uint32_t address, uint8_t* buffer, size_t size) {
  static constexpr uint8_t cmdSize = 4;
  uint8_t cmd[cmdSize] = {static_cast<uint8_t>(Commands::Read),
                          static_cast<uint8_t>(address >> 16U),
                          static_cast<uint8_t>(address >> 8U),
                          static_cast<uint8_t>(address)};
  spi.Read(reinterpret_cast<uint8_t*>(&cmd), cmdSize, buffer, size);
}

void SpiNorFlash::WriteEnable() {
  auto cmd = static_cast<uint8_t>(Commands::WriteEnable);
  spi.Read(&cmd, sizeof(cmd), nullptr, 0);
}

void SpiNorFlash::SectorErase(uint32_t sectorAddress) {
  static constexpr uint8_t cmdSize = 4;
  uint8_t cmd[cmdSize] = {static_cast<uint8_t>(Commands::SectorErase),
                          static_cast<uint8_t>(sectorAddress >> 16U),
                          static_cast<uint8_t>(sectorAddress >> 8U),
                          static_cast<uint8_t>(sectorAddress)};

  WriteEnable();
  while (!WriteEnabled())
    vTaskDelay(1);

  spi.Read(reinterpret_cast<uint8_t*>(&cmd), cmdSize, nullptr, 0);

  while (WriteInProgress())
    vTaskDelay(1);
}

uint8_t SpiNorFlash::ReadSecurityRegister() {
  auto cmd = static_cast<uint8_t>(Commands::ReadSecurityRegister);
  uint8_t status;
  spi.Read(&cmd, sizeof(cmd), &status, sizeof(uint8_t));
  return status;
}

bool SpiNorFlash::ProgramFailed() {
  return (ReadSecurityRegister() & 0x20u) == 0x20u;
}

bool SpiNorFlash::EraseFailed() {
  return (ReadSecurityRegister() & 0x40u) == 0x40u;
}

void SpiNorFlash::Write(uint32_t address, const uint8_t* buffer, size_t size) {
  static constexpr uint8_t cmdSize = 4;

  size_t len = size;
  uint32_t addr = address;
  const uint8_t* b = buffer;
  while (len > 0) {
    uint32_t pageLimit = (addr & ~(pageSize - 1u)) + pageSize;
    uint32_t toWrite = pageLimit - addr > len ? len : pageLimit - addr;

    uint8_t cmd[cmdSize] = {static_cast<uint8_t>(Commands::PageProgram),
                            static_cast<uint8_t>(addr >> 16U),
                            static_cast<uint8_t>(addr >> 8U),
                            static_cast<uint8_t>(addr)};

    WriteEnable();
    while (!WriteEnabled())
      vTaskDelay(1);

    spi.WriteCmdAndBuffer(cmd, cmdSize, b, toWrite);

    while (WriteInProgress())
      vTaskDelay(1);

    addr += toWrite;
    b += toWrite;
    len -= toWrite;
  }
}
