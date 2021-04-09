#include "TwiMaster.h"
#include <cstring>
#include <hal/nrf_gpio.h>
#include <nrfx_log.h>
#include <nrfx_twim.h>
#include <nrf_drv_twi.h>
using namespace Pinetime::Drivers;

// TODO use shortcut to automatically send STOP when receive LastTX, for example
// TODO use DMA/IRQ

TwiMaster::TwiMaster(const Modules module, const Parameters& params) : module{module}, params{params}, mutex{xSemaphoreCreateBinary()} {
  ASSERT(mutex != nullptr);
  switch(module) {
    case Modules::TWIM1:
    default:
      twim = NRFX_TWIM_INSTANCE(1);
      break;
  }
}

void TwiMaster::Init() {
  nrfx_twim_config_t config;
  config.frequency = static_cast<nrf_twim_frequency_t>(params.frequency);
  config.hold_bus_uninit = false;
  config.interrupt_priority = 0;
  config.scl = params.pinScl;
  config.sda = params.pinSda;
  nrfx_twim_init(&twim,
                 &config,
                 nullptr,
                 nullptr);
  nrfx_twim_enable(&twim);

  xSemaphoreGive(mutex);
}

TwiMaster::ErrorCodes TwiMaster::Read(uint8_t deviceAddress, uint8_t registerAddress, uint8_t *data, size_t size) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  TwiMaster::ErrorCodes ret;

  auto err = nrfx_twim_tx(&twim, deviceAddress, &registerAddress, 1, false);
  if(err != 0) {
    return TwiMaster::ErrorCodes::TransactionFailed;
  }

  err = nrfx_twim_rx(&twim, deviceAddress, data, size);
  if(err != 0) {
    return TwiMaster::ErrorCodes::TransactionFailed;
  }
  xSemaphoreGive(mutex);

  return TwiMaster::ErrorCodes::NoError;
}

TwiMaster::ErrorCodes TwiMaster::Write(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t *data, size_t size) {
  ASSERT(size <= maxDataSize);
  xSemaphoreTake(mutex, portMAX_DELAY);
  TwiMaster::ErrorCodes ret;

  internalBuffer[0] = registerAddress;
  std::memcpy(internalBuffer+1, data, size);
  auto err = nrfx_twim_tx(&twim, deviceAddress, internalBuffer , size+1, false);
  if(err != 0){
    return TwiMaster::ErrorCodes::TransactionFailed;
  }

  xSemaphoreGive(mutex);
  return TwiMaster::ErrorCodes::NoError;
}

void TwiMaster::Sleep() {
  nrfx_twim_disable(&twim);
  nrfx_twim_uninit(&twim);

  nrf_gpio_cfg_default(6);
  nrf_gpio_cfg_default(7);
  NRF_LOG_INFO("[TWIMASTER] Sleep");
}

void TwiMaster::Wakeup() {
  Init();
  NRF_LOG_INFO("[TWIMASTER] Wakeup");
}
