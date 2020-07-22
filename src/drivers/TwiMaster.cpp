#include <nrfx_log.h>
#include <hal/nrf_gpio.h>
#include <cstring>
#include "TwiMaster.h"

using namespace Pinetime::Drivers;

// TODO use shortcut to automatically send STOP when receive LastTX, for example
// TODO use DMA/IRQ

TwiMaster::TwiMaster(const Modules module, const Parameters& params) : module{module}, params{params} {
  mutex = xSemaphoreCreateBinary();
}

void TwiMaster::Init() {
  NRF_GPIO->PIN_CNF[params.pinScl] = ((uint32_t)GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos)
                                     | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                                     | ((uint32_t)GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)
                                     | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1       << GPIO_PIN_CNF_DRIVE_Pos)
                                     | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);

  NRF_GPIO->PIN_CNF[params.pinSda] = ((uint32_t)GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos)
                                     | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos)
                                     | ((uint32_t)GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)
                                     | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1       << GPIO_PIN_CNF_DRIVE_Pos)
                                     | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos);

  switch(module) {
    case Modules::TWIM1: twiBaseAddress = NRF_TWIM1; break;
    default:
      return;
  }

  switch(static_cast<Frequencies>(params.frequency)) {
    case Frequencies::Khz100 : twiBaseAddress->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K100; break;
    case Frequencies::Khz250 : twiBaseAddress->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K250; break;
    case Frequencies::Khz400 : twiBaseAddress->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K400; break;
  }

  twiBaseAddress->PSEL.SCL = params.pinScl;
  twiBaseAddress->PSEL.SDA = params.pinSda;
  twiBaseAddress->EVENTS_LASTRX = 0;
  twiBaseAddress->EVENTS_STOPPED = 0;
  twiBaseAddress->EVENTS_LASTTX = 0;
  twiBaseAddress->EVENTS_ERROR = 0;
  twiBaseAddress->EVENTS_RXSTARTED = 0;
  twiBaseAddress->EVENTS_SUSPENDED = 0;
  twiBaseAddress->EVENTS_TXSTARTED = 0;

  twiBaseAddress->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);


  /* // IRQ
     NVIC_ClearPendingIRQ(_IRQn);
     NVIC_SetPriority(_IRQn, 2);
     NVIC_EnableIRQ(_IRQn);
   */

  xSemaphoreGive(mutex);

}

void TwiMaster::Read(uint8_t deviceAddress, uint8_t registerAddress, uint8_t *data, size_t size) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  Write(deviceAddress, &registerAddress, 1, false);
  Read(deviceAddress, data, size, true);
  xSemaphoreGive(mutex);
}

void TwiMaster::Write(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t *data, size_t size) {
  ASSERT(size <= maxDataSize);
  xSemaphoreTake(mutex, portMAX_DELAY);
  internalBuffer[0] = registerAddress;
  std::memcpy(internalBuffer+1, data, size);
  Write(deviceAddress, internalBuffer, size+1, true);
  xSemaphoreGive(mutex);
}


void TwiMaster::Read(uint8_t deviceAddress, uint8_t *buffer, size_t size, bool stop) {
  twiBaseAddress->ADDRESS = deviceAddress;
  twiBaseAddress->TASKS_RESUME = 0x1UL;
  twiBaseAddress->RXD.PTR = (uint32_t)buffer;
  twiBaseAddress->RXD.MAXCNT = size;

  twiBaseAddress->TASKS_STARTRX = 1;

  while(!twiBaseAddress->EVENTS_RXSTARTED && !twiBaseAddress->EVENTS_ERROR);
  twiBaseAddress->EVENTS_RXSTARTED = 0x0UL;

  while(!twiBaseAddress->EVENTS_LASTRX && !twiBaseAddress->EVENTS_ERROR);
  twiBaseAddress->EVENTS_LASTRX = 0x0UL;

  if (stop || twiBaseAddress->EVENTS_ERROR) {
    twiBaseAddress->TASKS_STOP = 0x1UL;
    while(!twiBaseAddress->EVENTS_STOPPED);
    twiBaseAddress->EVENTS_STOPPED = 0x0UL;
  }
  else {
    twiBaseAddress->TASKS_SUSPEND = 0x1UL;
    while(!twiBaseAddress->EVENTS_SUSPENDED);
    twiBaseAddress->EVENTS_SUSPENDED = 0x0UL;
  }

  if (twiBaseAddress->EVENTS_ERROR) {
    twiBaseAddress->EVENTS_ERROR = 0x0UL;
  }
}

void TwiMaster::Write(uint8_t deviceAddress, const uint8_t *data, size_t size, bool stop) {
  twiBaseAddress->ADDRESS = deviceAddress;
  twiBaseAddress->TASKS_RESUME = 0x1UL;
  twiBaseAddress->TXD.PTR = (uint32_t)data;
  twiBaseAddress->TXD.MAXCNT = size;

  twiBaseAddress->TASKS_STARTTX = 1;

  while(!twiBaseAddress->EVENTS_TXSTARTED && !twiBaseAddress->EVENTS_ERROR);
  twiBaseAddress->EVENTS_TXSTARTED = 0x0UL;

  while(!twiBaseAddress->EVENTS_LASTTX && !twiBaseAddress->EVENTS_ERROR);
  twiBaseAddress->EVENTS_LASTTX = 0x0UL;

  if (stop || twiBaseAddress->EVENTS_ERROR) {
    twiBaseAddress->TASKS_STOP = 0x1UL;
    while(!twiBaseAddress->EVENTS_STOPPED);
    twiBaseAddress->EVENTS_STOPPED = 0x0UL;
  }
  else {
    twiBaseAddress->TASKS_SUSPEND = 0x1UL;
    while(!twiBaseAddress->EVENTS_SUSPENDED);
    twiBaseAddress->EVENTS_SUSPENDED = 0x0UL;
  }

  if (twiBaseAddress->EVENTS_ERROR) {
    twiBaseAddress->EVENTS_ERROR = 0x0UL;
    uint32_t error = twiBaseAddress->ERRORSRC;
    twiBaseAddress->ERRORSRC = error;
  }
}