#include <drivers/include/nrfx_saadc.h>
#include <hal/nrf_gpio.h>
#include <libraries/log/nrf_log.h>
#include "BatteryController.h"

using namespace Pinetime::Controllers;

void Battery::Init() {
  nrf_gpio_cfg_input(12, (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pullup);
  nrf_gpio_cfg_input(19, (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pullup);

  nrfx_saadc_config_t adcConfig = NRFX_SAADC_DEFAULT_CONFIG;
  nrfx_saadc_init(&adcConfig, SaadcEventHandler);
  nrf_saadc_channel_config_t adcChannelConfig = {
          .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
          .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
          .gain       = NRF_SAADC_GAIN1_5,
          .reference  = NRF_SAADC_REFERENCE_INTERNAL,
          .acq_time   = NRF_SAADC_ACQTIME_3US,
          .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
          .burst      = NRF_SAADC_BURST_DISABLED,
          .pin_p      = (nrf_saadc_input_t)(SAADC_CH_PSELP_PSELP_AnalogInput7),
          .pin_n      = NRF_SAADC_INPUT_DISABLED
  };
  nrfx_saadc_channel_init(0, &adcChannelConfig);
}

void Battery::Update() {
  isCharging = !nrf_gpio_pin_read(12);
  isPowerPresent = !nrf_gpio_pin_read(19);

  nrf_saadc_value_t value = 0;
  nrfx_saadc_sample_convert(0, &value);

  voltage = (value * 2.0f) / (1024/3.0f);
  percentRemaining = ((voltage - 3.55)*100)*3.9;

  NRF_LOG_INFO("BATTERY " NRF_LOG_FLOAT_MARKER " %% - " NRF_LOG_FLOAT_MARKER " v", NRF_LOG_FLOAT(percentRemaining), NRF_LOG_FLOAT(voltage));
  NRF_LOG_INFO("POWER Charging : %d - Power : %d", isCharging, isPowerPresent);
}

void Battery::SaadcEventHandler(nrfx_saadc_evt_t const * event) {

}