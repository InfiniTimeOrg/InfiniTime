#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <nrfx_gpiote.h>

void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void DebounceTimerCallback(TimerHandle_t xTimer);

extern int mallocFailedCount;
extern int stackOverflowCount;