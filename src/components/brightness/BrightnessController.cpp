#include "components/brightness/BrightnessController.h"
#include <hal/nrf_gpio.h>
#include "displayapp/screens/Symbols.h"
#include "drivers/PinMap.h"
#include <libraries/delay/nrf_delay.h>
using namespace Pinetime::Controllers;

namespace {
  // reinterpret_cast is not constexpr so this is the best we can do
  static NRF_RTC_Type* const RTC = reinterpret_cast<NRF_RTC_Type*>(NRF_RTC2_BASE);
}

void BrightnessController::Init() {
  nrf_gpio_cfg_output(PinMap::LcdBacklightLow);
  nrf_gpio_cfg_output(PinMap::LcdBacklightMedium);
  nrf_gpio_cfg_output(PinMap::LcdBacklightHigh);

  nrf_gpio_pin_clear(PinMap::LcdBacklightLow);
  nrf_gpio_pin_clear(PinMap::LcdBacklightMedium);
  nrf_gpio_pin_clear(PinMap::LcdBacklightHigh);

  static_assert(timerFrequency == 32768, "Change the prescaler below");
  RTC->PRESCALER = 0;
  // CC1 switches the backlight on (pin transitions from high to low) and resets the counter to 0
  RTC->CC[1] = timerPeriod;
  // Enable compare events for CC0,CC1
  RTC->EVTEN = 0b0000'0000'0000'0011'0000'0000'0000'0000;
  // Disable all interrupts
  RTC->INTENCLR = 0b0000'0000'0000'1111'0000'0000'0000'0011;
  Set(level);
}

void BrightnessController::ApplyBrightness(uint16_t rawBrightness) {
  // The classic off, low, medium, high brightnesses are at {0, timerPeriod, timerPeriod*2, timerPeriod*3}
  // These brightness levels do not use PWM: they only set/clear the corresponding pins
  // Any brightness level between the above levels is achieved with efficient RTC based PWM on the next pin up
  // E.g 2.5*timerPeriod corresponds to medium brightness with 50% PWM on the high pin
  // Note: Raw brightness does not necessarily correspond to a linear perceived brightness

  uint8_t pin;
  if (rawBrightness > 2 * timerPeriod) {
    rawBrightness -= 2 * timerPeriod;
    pin = PinMap::LcdBacklightHigh;
  } else if (rawBrightness > timerPeriod) {
    rawBrightness -= timerPeriod;
    pin = PinMap::LcdBacklightMedium;
  } else {
    pin = PinMap::LcdBacklightLow;
  }
  if (rawBrightness == timerPeriod || rawBrightness == 0) {
    if (lastPin != UNSET) {
      RTC->TASKS_STOP = 1;
      nrf_delay_us(rtcStopTime);
      nrf_ppi_channel_disable(ppiBacklightOff);
      nrf_ppi_channel_disable(ppiBacklightOn);
      nrfx_gpiote_out_uninit(lastPin);
      nrf_gpio_cfg_output(lastPin);
    }
    lastPin = UNSET;
    if (rawBrightness == 0) {
      nrf_gpio_pin_set(pin);
    } else {
      nrf_gpio_pin_clear(pin);
    }
  } else {
    // If the pin on which we are doing PWM is changing
    // Disable old PWM channel (if exists) and set up new one
    if (lastPin != pin) {
      if (lastPin != UNSET) {
        RTC->TASKS_STOP = 1;
        nrf_delay_us(rtcStopTime);
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
                                     reinterpret_cast<uint32_t>(&RTC->EVENTS_COMPARE[0]),
                                     nrfx_gpiote_out_task_addr_get(pin));
      nrf_ppi_channel_endpoint_setup(ppiBacklightOn,
                                     reinterpret_cast<uint32_t>(&RTC->EVENTS_COMPARE[1]),
                                     nrfx_gpiote_out_task_addr_get(pin));
      nrf_ppi_fork_endpoint_setup(ppiBacklightOn, reinterpret_cast<uint32_t>(&RTC->TASKS_CLEAR));
      nrf_ppi_channel_enable(ppiBacklightOff);
      nrf_ppi_channel_enable(ppiBacklightOn);
    } else {
      // If the pin used for PWM isn't changing, we only need to set the pin state to the initial value (low)
      RTC->TASKS_STOP = 1;
      nrf_delay_us(rtcStopTime);
      // Due to errata 20,179 and the intricacies of RTC timing, keep it simple: override the pin state
      nrfx_gpiote_out_task_force(pin, false);
    }
    // CC0 switches the backlight off (pin transitions from low to high)
    RTC->CC[0] = rawBrightness;
    RTC->TASKS_CLEAR = 1;
    RTC->TASKS_START = 1;
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
      ApplyBrightness(3 * timerPeriod);
      break;
    case Levels::Medium:
      ApplyBrightness(2 * timerPeriod);
      break;
    case Levels::Low:
      ApplyBrightness(timerPeriod);
      break;
    case Levels::AlwaysOn:
      ApplyBrightness(timerPeriod / 10);
      break;
    case Levels::Off:
      ApplyBrightness(0);
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
