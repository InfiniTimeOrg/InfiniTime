#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"
#include "nrf_drv_pwm.h"

using namespace Pinetime::Controllers;

static nrf_drv_pwm_t m_pwm2 = NRF_DRV_PWM_INSTANCE(2);
static nrf_pwm_values_individual_t seq_values;
static nrf_pwm_sequence_t const seq = {
    {
        .p_individual = &seq_values
    },
    NRF_PWM_VALUES_LENGTH(seq_values),
    0,
    0
};

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  shortVib = xTimerCreate("shortVib", 1, pdFALSE, nullptr, StopMotor);
  longVib = xTimerCreate("longVib", pdMS_TO_TICKS(1000), pdTRUE, this, Ring);
  alarmVib = xTimerCreate("alarmVib", pdMS_TO_TICKS(500), pdTRUE, this, AlarmRing);
  //gradualWakeBuzzDelay = xTimerCreate("gradualWakeBuzzDelay", pdMS_TO_TICKS(15), pdTRUE, this, GradualWakeBuzzRing);
  //gradualWakeBuzzEnd = xTimerCreate("gradualWakeBuzzEnd", pdMS_TO_TICKS(550), pdTRUE, this, StopGradualWakeBuzzCallback);

  InitPWM();
}

void MotorController::InitPWM() {
  nrf_drv_pwm_config_t const config2 = {
      .output_pins = {
          PinMap::Motor, // channel 0
          NRF_DRV_PWM_PIN_NOT_USED, // channel 1
          NRF_DRV_PWM_PIN_NOT_USED, // channel 2
          NRF_DRV_PWM_PIN_NOT_USED  // channel 3
      },
      .irq_priority = APP_IRQ_PRIORITY_LOWEST,
      .base_clock = NRF_PWM_CLK_1MHz,
      .count_mode = NRF_PWM_MODE_UP,
      .top_value = 100,
      .load_mode = NRF_PWM_LOAD_INDIVIDUAL,
      .step_mode = NRF_PWM_STEP_AUTO
  };

  nrf_drv_pwm_init(&m_pwm2, &config2, NULL);
  seq_values.channel_0 = 0;
  nrf_drv_pwm_simple_playback(&m_pwm2, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);

  SetMotorStrength(100);
}

void MotorController::SetMotorStrength(uint8_t strength) {
  if (strength > 100) strength = 100;
  seq_values.channel_0 = strength;
}

void MotorController::Ring(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

void MotorController::AlarmRing(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(300);
}

void MotorController::RunForDuration(uint16_t motorDuration) {
  if (motorDuration > 0 && xTimerChangePeriod(shortVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(shortVib, 0) == pdPASS) {
    nrf_gpio_pin_clear(PinMap::Motor);
    nrf_drv_pwm_simple_playback(&m_pwm2, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);
  }
}

void MotorController::StartRinging() {
  SetMotorStrength(100);
  RunForDuration(50);
  xTimerStart(longVib, 0);
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
  nrf_drv_pwm_stop(&m_pwm2, true);
}

void MotorController::StartAlarm() {
  SetMotorStrength(100);
  RunForDuration(300);
  xTimerStart(alarmVib, 0);
}

void MotorController::StopAlarm() {
  xTimerStop(alarmVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
  nrf_drv_pwm_stop(&m_pwm2, true);
}

void MotorController::StopMotor(TimerHandle_t /*xTimer*/) {
  nrf_gpio_pin_set(PinMap::Motor);
  nrf_drv_pwm_stop(&m_pwm2, true);
}

void MotorController::GradualWakeBuzz() {
  SetMotorStrength(40);
  RunForDuration(540);
  //xTimerStart(gradualWakeBuzzDelay, 0);
  //xTimerStart(gradualWakeBuzzEnd, 0);
}

void MotorController::GradualWakeBuzzRing(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(12);
}

void MotorController::StopGradualWakeBuzzCallback(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->StopGradualWakeBuzz();
}

void MotorController::StopGradualWakeBuzz() {
  //xTimerStop(gradualWakeBuzzDelay, 0);
  xTimerStop(gradualWakeBuzzEnd, 0);
  SetMotorStrength(100);
  StopMotor(nullptr);
}
