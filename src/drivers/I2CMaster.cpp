#include <legacy/nrf_drv_gpiote.h>

#include "I2CMaster.h"

using namespace Pinetime::Drivers;

Pinetime::Drivers::I2CMaster::I2CMaster() {
    twi = NRFX_TWI_INSTANCE(1);
    config.frequency = NRF_TWI_FREQ_400K;
    config.scl = 7;
    config.sda = 6;
    config.interrupt_priority = NRFX_TWI_DEFAULT_CONFIG_IRQ_PRIORITY;
    config.hold_bus_uninit = NRFX_TWI_DEFAULT_CONFIG_HOLD_BUS_UNINIT;
}

void Pinetime::Drivers::I2CMaster::Init() {
    auto ret = nrfx_twi_init(&twi, &config, nullptr, this);
    nrfx_twi_enable(&twi);
}

void Pinetime::Drivers::I2CMaster::Sleep() {
    nrfx_twi_disable(&twi);
    nrf_gpio_cfg_default(6);
    nrf_gpio_cfg_default(7);
}

void Pinetime::Drivers::I2CMaster::Wakeup() {
    Init();
}

void I2CMaster::Read(const uint8_t address, uint8_t *buffer, size_t size) {
    nrfx_twi_rx(&twi, address, buffer, size);
}
