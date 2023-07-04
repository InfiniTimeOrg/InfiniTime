#include "components/battery/BatteryController.h"
#include "utility/LinearApproximation.h"
#include "drivers/PinMap.h"
#include <hal/nrf_gpio.h>
#include <nrfx_saadc.h>
#include <algorithm>
#include <cmath>

using namespace Pinetime::Controllers;

Battery* Battery::instance = nullptr;

Battery::Battery() {
  instance = this;
  nrf_gpio_cfg_input(PinMap::Charging, static_cast<nrf_gpio_pin_pull_t> GPIO_PIN_CNF_PULL_Disabled);
}

void Battery::ReadPowerState() {
  isCharging = (nrf_gpio_pin_read(PinMap::Charging) == 0);
  isPowerPresent = (nrf_gpio_pin_read(PinMap::PowerPresent) == 0);

  if (isPowerPresent && !isCharging) {
    isFull = true;
  } else if (!isPowerPresent) {
    isFull = false;
  }
}

void Battery::MeasureVoltage() {
  ReadPowerState();

  if (isReading) {
    return;
  }
  // Non blocking read
  isReading = true;
  SaadcInit();

  nrfx_saadc_sample();
}

void Battery::AdcCallbackStatic(nrfx_saadc_evt_t const* event) {
  instance->SaadcEventHandler(event);
}

void Battery::SaadcInit() {
  nrfx_saadc_config_t adcConfig = NRFX_SAADC_DEFAULT_CONFIG;
  APP_ERROR_CHECK(nrfx_saadc_init(&adcConfig, AdcCallbackStatic));

  nrf_saadc_channel_config_t adcChannelConfig = {.resistor_p = NRF_SAADC_RESISTOR_DISABLED,
                                                 .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
                                                 .gain = NRF_SAADC_GAIN1_4,
                                                 .reference = NRF_SAADC_REFERENCE_INTERNAL,
                                                 .acq_time = NRF_SAADC_ACQTIME_40US,
                                                 .mode = NRF_SAADC_MODE_SINGLE_ENDED,
                                                 .burst = NRF_SAADC_BURST_ENABLED,
                                                 .pin_p = batteryVoltageAdcInput,
                                                 .pin_n = NRF_SAADC_INPUT_DISABLED};
  APP_ERROR_CHECK(nrfx_saadc_channel_init(0, &adcChannelConfig));
  APP_ERROR_CHECK(nrfx_saadc_buffer_convert(&saadc_value, 1));
}

void Battery::SaadcEventHandler(nrfx_saadc_evt_t const* p_event) {
  static const Utility::LinearApproximation<uint16_t, uint8_t, 6> approx {
    {{{3500, 0}, {3616, 3}, {3723, 22}, {3776, 48}, {3979, 79}, {4180, 100}}}};

  if (p_event->type == NRFX_SAADC_EVT_DONE) {

    APP_ERROR_CHECK(nrfx_saadc_buffer_convert(&saadc_value, 1));

    // A hardware voltage divider divides the battery voltage by 2
    // ADC gain is 1/4
    // thus adc_voltage = battery_voltage / 2 * gain = battery_voltage / 8
    // reference_voltage is 600mV
    // p_event->data.done.p_buffer[0] = (adc_voltage / reference_voltage) * 1024
    voltage = p_event->data.done.p_buffer[0] * (8 * 600) / 1024;

    uint8_t newPercent = 100;
    if (!isFull) {
      // max. voltage while charging is higher than when discharging
      newPercent = std::min(approx.GetValue(voltage), isCharging ? uint8_t {99} : uint8_t {100});
    }

    if ((isPowerPresent && newPercent > percentRemaining) || (!isPowerPresent && newPercent < percentRemaining) || firstMeasurement) {
      firstMeasurement = false;
      percentRemaining = newPercent;
      systemTask->PushMessage(System::Messages::BatteryPercentageUpdated);
    }

    nrfx_saadc_uninit();
    isReading = false;
  }
}

void Battery::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}
