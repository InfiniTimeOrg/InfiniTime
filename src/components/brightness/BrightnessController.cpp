#include "components/brightness/BrightnessController.h"
#include <hal/nrf_gpio.h>
#include "displayapp/screens/Symbols.h"
#include "drivers/PinMap.h"

#include "nrf_pwm.h"

using namespace Pinetime::Controllers;

void BrightnessController::Init() {
  nrf_gpio_cfg_output(PinMap::LcdBacklightLow);
  nrf_gpio_cfg_output(PinMap::LcdBacklightMedium);
  nrf_gpio_cfg_output(PinMap::LcdBacklightHigh);

  nrf_gpio_pin_clear(PinMap::LcdBacklightLow);
  nrf_gpio_pin_clear(PinMap::LcdBacklightMedium);
  nrf_gpio_pin_clear(PinMap::LcdBacklightHigh);

  static nrf_pwm_sequence_t seq;

  seq.values.p_common = pwmSequence;
  seq.length          = 1;
  seq.repeats         = 0;
  seq.end_delay       = 0;

  uint32_t out_pins[] = {PinMap::LcdBacklightHigh, PinMap::LcdBacklightMedium, PinMap::LcdBacklightLow, NRF_PWM_PIN_NOT_CONNECTED};

  nrf_pwm_pins_set(NRF_PWM0, out_pins);
  nrf_pwm_enable(NRF_PWM0);
  // With 8 MHz and 10000 reload timer PWM frequency is 712 Hz
  nrf_pwm_configure(NRF_PWM0, NRF_PWM_CLK_8MHz, NRF_PWM_MODE_UP, 10000);
  nrf_pwm_loop_set(NRF_PWM0, 0);
  nrf_pwm_decoder_set(NRF_PWM0, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
  nrf_pwm_sequence_set(NRF_PWM0, 0, &seq);
  nrf_pwm_task_trigger(NRF_PWM0, NRF_PWM_TASK_SEQSTART0);

  pwmVal = 0;
  Set(level);
};

void BrightnessController::setPwm(uint16_t val) {
  pwmSequence[0] = val;
  nrf_pwm_task_trigger(NRF_PWM0, NRF_PWM_TASK_SEQSTART0);
};

uint16_t BrightnessController::getPwm(BrightnessController::Levels level) {
  this->level = level;
  switch (level) {
    default:
    case Levels::High:
      return 10000;
    case Levels::Medium:
      return 4000;
    case Levels::Low:
      return 900;
    case Levels::Off:
      return 0;
  }
}

void BrightnessController::Set(BrightnessController::Levels level) {
  this->level = level;
  uint16_t target = getPwm(level);
  uint16_t step = abs((pwmVal - target) / 10);

  while (target != pwmVal) {
    if (target > pwmVal) {
      pwmVal += step;
    } else {
      pwmVal -= step;
    }
    setPwm(pwmVal);
    vTaskDelay(20);
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
