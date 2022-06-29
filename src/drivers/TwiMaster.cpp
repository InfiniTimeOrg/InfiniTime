#include "drivers/TwiMaster.h"
#include <cstring>
#include <hal/nrf_gpio.h>
#include <nrfx_log.h>

using namespace Pinetime::Drivers;

// TODO use shortcut to automatically send STOP when receive LastTX, for example
// TODO use DMA/IRQ

TwiMaster::TwiMaster(NRF_TWIM_Type* module, uint32_t frequency, uint8_t pinSda, uint8_t pinScl)
  : module {module}, frequency {frequency}, pinSda {pinSda}, pinScl {pinScl} {
}

void TwiMaster::ConfigurePins() const {
  NRF_GPIO->PIN_CNF[pinScl] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                              (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) |
                              (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

  NRF_GPIO->PIN_CNF[pinSda] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                              (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) | (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) |
                              (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}

void TwiMaster::Init() {
  if (mutex == nullptr) {
    mutex = xSemaphoreCreateBinary();
  }

  ConfigurePins();

  twiBaseAddress = module;

  twiBaseAddress->FREQUENCY = frequency;

  twiBaseAddress->PSEL.SCL = pinScl;
  twiBaseAddress->PSEL.SDA = pinSda;
  twiBaseAddress->EVENTS_LASTRX = 0;
  twiBaseAddress->EVENTS_STOPPED = 0;
  twiBaseAddress->EVENTS_LASTTX = 0;
  twiBaseAddress->EVENTS_ERROR = 0;
  twiBaseAddress->EVENTS_RXSTARTED = 0;
  twiBaseAddress->EVENTS_SUSPENDED = 0;
  twiBaseAddress->EVENTS_TXSTARTED = 0;

  twiBaseAddress->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);

  xSemaphoreGive(mutex);
}

TwiMaster::ErrorCodes TwiMaster::Read(uint8_t deviceAddress, uint8_t registerAddress, uint8_t* data, size_t size) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  Wakeup();
  auto ret = Write(deviceAddress, &registerAddress, 1, false);
  ret = Read(deviceAddress, data, size, true);
  Sleep();
  xSemaphoreGive(mutex);
  return ret;
}

TwiMaster::ErrorCodes TwiMaster::Write(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t* data, size_t size) {
  ASSERT(size <= maxDataSize);
  xSemaphoreTake(mutex, portMAX_DELAY);
  Wakeup();
  internalBuffer[0] = registerAddress;
  std::memcpy(internalBuffer + 1, data, size);
  auto ret = Write(deviceAddress, internalBuffer, size + 1, true);
  Sleep();
  xSemaphoreGive(mutex);
  return ret;
}

TwiMaster::ErrorCodes TwiMaster::Read(uint8_t deviceAddress, uint8_t* buffer, size_t size, bool stop) {
  twiBaseAddress->ADDRESS = deviceAddress;
  twiBaseAddress->TASKS_RESUME = 0x1UL;
  twiBaseAddress->RXD.PTR = (uint32_t) buffer;
  twiBaseAddress->RXD.MAXCNT = size;

  twiBaseAddress->TASKS_STARTRX = 1;

  while (!twiBaseAddress->EVENTS_RXSTARTED && !twiBaseAddress->EVENTS_ERROR)
    ;
  twiBaseAddress->EVENTS_RXSTARTED = 0x0UL;

  txStartedCycleCount = DWT->CYCCNT;
  uint32_t currentCycleCount;
  while (!twiBaseAddress->EVENTS_LASTRX && !twiBaseAddress->EVENTS_ERROR) {
    currentCycleCount = DWT->CYCCNT;
    if ((currentCycleCount - txStartedCycleCount) > HwFreezedDelay) {
      FixHwFreezed();
      return ErrorCodes::TransactionFailed;
    }
  }
  twiBaseAddress->EVENTS_LASTRX = 0x0UL;

  if (stop || twiBaseAddress->EVENTS_ERROR) {
    twiBaseAddress->TASKS_STOP = 0x1UL;
    while (!twiBaseAddress->EVENTS_STOPPED)
      ;
    twiBaseAddress->EVENTS_STOPPED = 0x0UL;
  } else {
    twiBaseAddress->TASKS_SUSPEND = 0x1UL;
    while (!twiBaseAddress->EVENTS_SUSPENDED)
      ;
    twiBaseAddress->EVENTS_SUSPENDED = 0x0UL;
  }

  if (twiBaseAddress->EVENTS_ERROR) {
    twiBaseAddress->EVENTS_ERROR = 0x0UL;
  }
  return ErrorCodes::NoError;
}

TwiMaster::ErrorCodes TwiMaster::Write(uint8_t deviceAddress, const uint8_t* data, size_t size, bool stop) {
  twiBaseAddress->ADDRESS = deviceAddress;
  twiBaseAddress->TASKS_RESUME = 0x1UL;
  twiBaseAddress->TXD.PTR = (uint32_t) data;
  twiBaseAddress->TXD.MAXCNT = size;

  twiBaseAddress->TASKS_STARTTX = 1;

  while (!twiBaseAddress->EVENTS_TXSTARTED && !twiBaseAddress->EVENTS_ERROR)
    ;
  twiBaseAddress->EVENTS_TXSTARTED = 0x0UL;

  txStartedCycleCount = DWT->CYCCNT;
  uint32_t currentCycleCount;
  while (!twiBaseAddress->EVENTS_LASTTX && !twiBaseAddress->EVENTS_ERROR) {
    currentCycleCount = DWT->CYCCNT;
    if ((currentCycleCount - txStartedCycleCount) > HwFreezedDelay) {
      FixHwFreezed();
      return ErrorCodes::TransactionFailed;
    }
  }
  twiBaseAddress->EVENTS_LASTTX = 0x0UL;

  if (stop || twiBaseAddress->EVENTS_ERROR) {
    twiBaseAddress->TASKS_STOP = 0x1UL;
    while (!twiBaseAddress->EVENTS_STOPPED)
      ;
    twiBaseAddress->EVENTS_STOPPED = 0x0UL;
  } else {
    twiBaseAddress->TASKS_SUSPEND = 0x1UL;
    while (!twiBaseAddress->EVENTS_SUSPENDED)
      ;
    twiBaseAddress->EVENTS_SUSPENDED = 0x0UL;
  }

  if (twiBaseAddress->EVENTS_ERROR) {
    twiBaseAddress->EVENTS_ERROR = 0x0UL;
    uint32_t error = twiBaseAddress->ERRORSRC;
    twiBaseAddress->ERRORSRC = error;
  }

  return ErrorCodes::NoError;
}

void TwiMaster::Sleep() {
  twiBaseAddress->ENABLE = (TWIM_ENABLE_ENABLE_Disabled << TWIM_ENABLE_ENABLE_Pos);
}

void TwiMaster::Wakeup() {
  twiBaseAddress->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);
}

/* Sometimes, the TWIM device just freeze and never set the event EVENTS_LASTTX.
 * This method disable and re-enable the peripheral so that it works again.
 * This is just a workaround, and it would be better if we could find a way to prevent
 * this issue from happening.
 * */
void TwiMaster::FixHwFreezed() {
  NRF_LOG_INFO("I2C device frozen, reinitializing it!");

  uint32_t twi_state = NRF_TWI1->ENABLE;

  Sleep();

  twiBaseAddress->ENABLE = twi_state;
}
