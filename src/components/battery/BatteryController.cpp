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
  nrf_gpio_cfg_input(chargingPin, static_cast<nrf_gpio_pin_pull_t> GPIO_PIN_CNF_PULL_Pullup);
}

void Battery::Update() {

  isCharging = !nrf_gpio_pin_read(chargingPin);
  isPowerPresent = !nrf_gpio_pin_read(powerPresentPin);

  if (isReading) {
    return;
  }
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

  const uint16_t battery_max = 4180; // maximum voltage of battery ( max charging voltage is 4.21 )
  const uint16_t battery_min = 3200; // minimum voltage of battery before shutdown ( depends on the battery )

  if (p_event->type == NRFX_SAADC_EVT_DONE) {

    APP_ERROR_CHECK(nrfx_saadc_buffer_convert(&saadc_value, 1));

    // A hardware voltage divider divides the battery voltage by 2
    // ADC gain is 1/5
    // thus adc_voltage = battery_voltage / 2 * gain = battery_voltage / 10
    // reference_voltage is 0.6V
    // p_event->data.done.p_buffer[0] = (adc_voltage / reference_voltage) * 1024
    voltage = p_event->data.done.p_buffer[0] * 6000 / 1024;

    percentRemaining = (voltage - battery_min) * 100 / (battery_max - battery_min);

    percentRemaining = std::max(percentRemaining, 0);
    percentRemaining = std::min(percentRemaining, 100);

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
