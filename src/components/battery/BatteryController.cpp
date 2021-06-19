#include "BatteryController.h"
#include <hal/nrf_gpio.h>
#include <nrfx_saadc.h>
#include <libraries/log/nrf_log.h>
#include <algorithm>
#include <math.h>

using namespace Pinetime::Controllers;

Battery* Battery::instance = nullptr;

Battery::Battery() {
  instance = this;
}

void Battery::Init() {
  nrf_gpio_cfg_input(chargingPin, (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Pullup);
}

void Battery::Update() {

  isCharging = !nrf_gpio_pin_read(chargingPin);
  isPowerPresent = !nrf_gpio_pin_read(powerPresentPin);

  if (isReading)
    return;
  // Non blocking read
  samples = 0;
  isReading = true;
  SaadcInit();

  nrfx_saadc_sample();
}

void Battery::adcCallbackStatic(nrfx_saadc_evt_t const* event) {
  instance->SaadcEventHandler(event);
}

void Battery::SaadcInit() {
  nrfx_saadc_config_t adcConfig = NRFX_SAADC_DEFAULT_CONFIG;
  APP_ERROR_CHECK(nrfx_saadc_init(&adcConfig, adcCallbackStatic));

  nrf_saadc_channel_config_t adcChannelConfig = {.resistor_p = NRF_SAADC_RESISTOR_DISABLED,
                                                 .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
                                                 .gain = NRF_SAADC_GAIN1_5,
                                                 .reference = NRF_SAADC_REFERENCE_INTERNAL,
                                                 .acq_time = NRF_SAADC_ACQTIME_3US,
                                                 .mode = NRF_SAADC_MODE_SINGLE_ENDED,
                                                 .burst = NRF_SAADC_BURST_ENABLED,
                                                 .pin_p = batteryVoltageAdcInput,
                                                 .pin_n = NRF_SAADC_INPUT_DISABLED};
  APP_ERROR_CHECK(nrfx_saadc_channel_init(0, &adcChannelConfig));
  APP_ERROR_CHECK(nrfx_saadc_buffer_convert(&saadc_value, 1));
}

void Battery::SaadcEventHandler(nrfx_saadc_evt_t const* p_event) {
  if (p_event->type == NRFX_SAADC_EVT_DONE) {

    APP_ERROR_CHECK(nrfx_saadc_buffer_convert(&saadc_value, 1));

    voltage = (static_cast<float>(p_event->data.done.p_buffer[0]) * 2.04f) / (1024 / 3.0f);
    voltage = roundf(voltage * 100) / 100;

    percentRemaining = GetPercentageFromVoltage(voltage);
    percentRemainingBuffer.insert(percentRemaining);

    samples++;
    if (samples > percentRemainingSamples) {
      nrfx_saadc_uninit();
      isReading = false;
    } else {
      nrfx_saadc_sample();
    }
  }
}

// Voltage-to-percentage conversion table. Indexed by voltage, where index 0 represents 2.98v, index 1
// represents 2.99v, and so on. Valid indices range from 0 to 120 (or 2.98v to 4.18v).
static const uint8_t scaledVoltageToPercentage[] = {
  0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,
  3,4,4,4,4,4,4,4,5,5,5,5,5,5,6,
  6,6,6,7,7,7,8,8,9,9,10,11,11,12,13,
  14,14,15,16,17,19,20,22,24,26,28,32,37,41,46,
  49,51,53,55,57,59,60,62,64,65,67,68,70,71,73,
  74,75,77,78,80,81,82,83,84,85,86,88,89,90,91,
  92,93,94,94,95,96,97,98,98,99,99,100,100,100,100,
  100
};

int Battery::GetPercentageFromVoltage(float voltage) {
  const int scaledVoltageMinimum = 298; // Minimum observed voltage (x100) of battery before shutdown.
  const int scaledVoltageMaximum = 418; // Maximum observed voltage (x100) of battery (when not charging).

  // Scale up the floating point voltage (assuming 2 decimal digits) to an integer.
  int scaledVoltage = static_cast<int>(voltage * 100);

  // If the voltage is not within range of our conversion table, return clamped percentage.
  if (scaledVoltage < scaledVoltageMinimum)
    return 0;
  if (scaledVoltage > scaledVoltageMaximum)
    return 100;

  // Calculate the index into the table and return the battery percentage corresponding to the voltage.
  int index = scaledVoltage - scaledVoltageMinimum;
  return scaledVoltageToPercentage[index];
}
