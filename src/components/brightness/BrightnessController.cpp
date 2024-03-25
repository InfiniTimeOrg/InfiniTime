#include "components/brightness/BrightnessController.h"
#include <hal/nrf_gpio.h>
#include "displayapp/screens/Symbols.h"
#include "drivers/PinMap.h"
using namespace Pinetime::Controllers;

void BrightnessController::Init() {
  nrf_gpio_cfg_output(PinMap::LcdBacklightLow);
  nrf_gpio_cfg_output(PinMap::LcdBacklightMedium);
  nrf_gpio_cfg_output(PinMap::LcdBacklightHigh);

  nrf_gpio_pin_clear(PinMap::LcdBacklightLow);
  nrf_gpio_pin_clear(PinMap::LcdBacklightMedium);
  nrf_gpio_pin_clear(PinMap::LcdBacklightHigh);

  timer = NRFX_TIMER_INSTANCE(1);
  nrfx_timer_config_t timerCfg = {.frequency = NRF_TIMER_FREQ_1MHz,
                                  .mode = NRF_TIMER_MODE_TIMER,
                                  .bit_width = NRF_TIMER_BIT_WIDTH_32,
                                  .interrupt_priority = 6,
                                  .p_context = nullptr};
  // Callback will never fire, use empty expression
  APP_ERROR_CHECK(nrfx_timer_init(&timer, &timerCfg, [](auto, auto) {
  }));
  nrfx_timer_extended_compare(&timer,
                              NRF_TIMER_CC_CHANNEL1,
                              nrfx_timer_us_to_ticks(&timer, 1000),
                              NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK,
                              false);
  Set(level);
}

void BrightnessController::applyBrightness(uint16_t val) {
  uint8_t pin;
  if (val > 2000) {
    val -= 2000;
    pin = PinMap::LcdBacklightHigh;
  } else if (val > 1000) {
    val -= 1000;
    pin = PinMap::LcdBacklightMedium;
  } else {
    pin = PinMap::LcdBacklightLow;
  }
  if (val == 1000 || val == 0) {
    if (lastPin != UNSET) {
      nrfx_timer_disable(&timer);
      nrfx_timer_clear(&timer);
      nrf_ppi_channel_disable(ppiBacklightOff);
      nrf_ppi_channel_disable(ppiBacklightOn);
      nrfx_gpiote_out_uninit(lastPin);
      nrf_gpio_cfg_output(lastPin);
    }
    lastPin = UNSET;
    if (val == 0) {
      nrf_gpio_pin_set(pin);
    } else {
      nrf_gpio_pin_clear(pin);
    }
  } else {
    uint32_t newThreshold = nrfx_timer_us_to_ticks(&timer, val);
    if (lastPin != pin) {
      if (lastPin != UNSET) {
        nrfx_timer_disable(&timer);
        nrfx_timer_clear(&timer);
        nrf_ppi_channel_disable(ppiBacklightOff);
        nrf_ppi_channel_disable(ppiBacklightOn);
        nrfx_gpiote_out_uninit(lastPin);
        nrf_gpio_cfg_output(lastPin);
      }
      nrfx_gpiote_out_config_t gpioteCfg = {.action = NRF_GPIOTE_POLARITY_TOGGLE,
                                            .init_state = NRF_GPIOTE_INITIAL_VALUE_LOW,
                                            .task_pin = true};
      APP_ERROR_CHECK(nrfx_gpiote_out_init(pin, &gpioteCfg));
      nrfx_gpiote_out_task_enable(pin);
      nrf_ppi_channel_endpoint_setup(ppiBacklightOff,
                                     nrfx_timer_event_address_get(&timer, NRF_TIMER_EVENT_COMPARE0),
                                     nrfx_gpiote_out_task_addr_get(pin));
      nrf_ppi_channel_endpoint_setup(ppiBacklightOn,
                                     nrfx_timer_event_address_get(&timer, NRF_TIMER_EVENT_COMPARE1),
                                     nrfx_gpiote_out_task_addr_get(pin));
      nrf_ppi_channel_enable(ppiBacklightOff);
      nrf_ppi_channel_enable(ppiBacklightOn);
    } else {
      // Pause the timer, check where it is before changing the threshold
      // If we have already triggered CC0 and then move CC0 into the future
      // without triggering CC1 first, the modulation will be inverted
      // (e.g on 100us off 900us becomes off 100us on 900us)
      nrfx_timer_pause(&timer);
      uint32_t currentCycle = nrfx_timer_capture(&timer, NRF_TIMER_CC_CHANNEL2);
      uint32_t oldThreshold = nrfx_timer_capture_get(&timer, NRF_TIMER_CC_CHANNEL0);
      // If the new threshold now in future and we have already triggered the old threshold, switch backlight back on
      if (currentCycle >= oldThreshold && newThreshold > currentCycle) {
        nrfx_gpiote_out_task_trigger(pin);
      }
    }
    nrfx_timer_compare(&timer, NRF_TIMER_CC_CHANNEL0, newThreshold, false);
    if (lastPin != pin) {
      nrfx_timer_enable(&timer);
    } else {
      nrfx_timer_resume(&timer);
    }
    lastPin = pin;
  }
  switch (pin) {
    case PinMap::LcdBacklightHigh:
      nrf_gpio_pin_clear(PinMap::LcdBacklightLow);
      nrf_gpio_pin_clear(PinMap::LcdBacklightMedium);
      break;
    case PinMap::LcdBacklightMedium:
      nrf_gpio_pin_clear(PinMap::LcdBacklightLow);
      nrf_gpio_pin_set(PinMap::LcdBacklightHigh);
      break;
    case PinMap::LcdBacklightLow:
      nrf_gpio_pin_set(PinMap::LcdBacklightMedium);
      nrf_gpio_pin_set(PinMap::LcdBacklightHigh);
  }
}

void BrightnessController::Set(BrightnessController::Levels level) {
  this->level = level;
  switch (level) {
    default:
    case Levels::High:
      applyBrightness(3000);
      break;
    case Levels::Medium:
      applyBrightness(2000);
      break;
    case Levels::Low:
      applyBrightness(1000);
      break;
    case Levels::AlwaysOn:
      applyBrightness(100);
      break;
    case Levels::Off:
      applyBrightness(0);
      break;
  }
}

void BrightnessController::Lower() {
  switch (level) {
    case Levels::High:
      Set(Levels::Medium);
      break;
    case Levels::Medium:
      Set(Levels::Low);
      break;
    case Levels::Low:
      Set(Levels::Off);
      break;
    default:
      break;
  }
}

void BrightnessController::Higher() {
  switch (level) {
    case Levels::Off:
      Set(Levels::Low);
      break;
    case Levels::Low:
      Set(Levels::Medium);
      break;
    case Levels::Medium:
      Set(Levels::High);
      break;
    default:
      break;
  }
}

BrightnessController::Levels BrightnessController::Level() const {
  return level;
}

void BrightnessController::Step() {
  switch (level) {
    case Levels::Low:
      Set(Levels::Medium);
      break;
    case Levels::Medium:
      Set(Levels::High);
      break;
    case Levels::High:
      Set(Levels::Low);
      break;
    default:
      break;
  }
}

const char* BrightnessController::GetIcon() {
  switch (level) {
    case Levels::Medium:
      return Applications::Screens::Symbols::brightnessMedium;
    case Levels::High:
      return Applications::Screens::Symbols::brightnessHigh;
    default:
      break;
  }
  return Applications::Screens::Symbols::brightnessLow;
}

const char* BrightnessController::ToString() {
  switch (level) {
    case Levels::Off:
      return "Off";
    case Levels::Low:
      return "Low";
    case Levels::Medium:
      return "Medium";
    case Levels::High:
      return "High";
    default:
      return "???";
  }
}
