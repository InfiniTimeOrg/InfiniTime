#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
  DebugPin0 = 27,
  DebugPin1 = 29,
  DebugPin2 = 20,
  DebugPin3 = 17,
  DebugPin4 = 11,
} debugpins_pins;

void debugpins_init();
void debugpins_set(debugpins_pins pin);
void debugpins_clear(debugpins_pins pin);
void debugpins_pulse(debugpins_pins pin);

#ifdef __cplusplus
}
#endif

