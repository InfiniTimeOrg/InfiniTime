#include "drivers/DebugPins.h"
#include <hal/nrf_gpio.h>

#ifdef USE_DEBUG_PINS
void debugpins_init() {
  nrf_gpio_cfg_output(DebugPin0);
  nrf_gpio_pin_clear(DebugPin0);

  nrf_gpio_cfg_output(DebugPin1);
  nrf_gpio_pin_clear(DebugPin1);

  nrf_gpio_cfg_output(DebugPin2);
  nrf_gpio_pin_clear(DebugPin2);

  nrf_gpio_cfg_output(DebugPin3);
  nrf_gpio_pin_clear(DebugPin3);

  nrf_gpio_cfg_output(DebugPin4);
  nrf_gpio_pin_clear(DebugPin4);
}

void debugpins_set(debugpins_pins pin) {
  nrf_gpio_pin_set(static_cast<uint32_t>(pin));
}

void debugpins_clear(debugpins_pins pin) {
  nrf_gpio_pin_clear(static_cast<uint32_t>(pin));
}

void debugpins_pulse(debugpins_pins pin) {
  nrf_gpio_pin_set(static_cast<uint32_t>(pin));
  nrf_gpio_pin_clear(static_cast<uint32_t>(pin));
}
#else
void debugpins_init() {
}

void debugpins_set(debugpins_pins pin) {
}

void debugpins_clear(debugpins_pins pin) {
}

void debugpins_pulse(debugpins_pins pin) {
}

#endif
