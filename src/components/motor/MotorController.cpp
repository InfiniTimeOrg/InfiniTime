#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"
#include "nrf_pwm.h"

using namespace Pinetime::Controllers;

static uint16_t pwmValue = 0; // Declare the variable for PWM value

void MotorController::Init() {
  // Configure the motor pin as an output
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  // Configure the PWM sequence
  static nrf_pwm_sequence_t seq;
  seq.values.p_common = &pwmValue; // Use the PWM value array
  seq.length = NRF_PWM_VALUES_LENGTH(pwmValue);
  seq.repeats = 0;
  seq.end_delay = 0;

  // Configure the PWM pins
  uint32_t out_pins[] = {PinMap::Motor, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};
  nrf_pwm_pins_set(NRF_PWM2, out_pins);

  // Enable and configure the PWM peripheral
  nrf_pwm_enable(NRF_PWM2);
  nrf_pwm_configure(NRF_PWM2, NRF_PWM_CLK_1MHz, NRF_PWM_MODE_UP, 255); // Top value determines the resolution
  nrf_pwm_loop_set(NRF_PWM2, 0);                                       // Infinite loop
  nrf_pwm_decoder_set(NRF_PWM2, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
  nrf_pwm_sequence_set(NRF_PWM2, 0, &seq);

  // Start the PWM with an initial value of 0
  pwmValue = 0;
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0);

  // Initialize timers for motor actions
  shortVib = xTimerCreate("shortVib", 1, pdFALSE, nullptr, StopMotor);
  longVib = xTimerCreate("longVib", pdMS_TO_TICKS(1000), pdTRUE, this, Ring);
  alarmVib = xTimerCreate("alarmVib", pdMS_TO_TICKS(500), pdTRUE, this, AlarmRing);
  wakeAlarmVib = xTimerCreate("wakeAlarmVib", pdMS_TO_TICKS(1000), pdTRUE, this, WakeAlarmRing);
}

void MotorController::SetMotorStrength(uint8_t strength) {
  // Ensure strength is within bounds (0-100)
  // if (strength > 100)
  //   strength = 100;

  // Map the strength to the PWM value (0-100 -> 0-top_value)
  // pwmValue = (strength * 255) / 100;
  pwmValue = strength;
}

void MotorController::Ring(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

void MotorController::AlarmRing(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->SetMotorStrength(80);
  motorController->RunForDuration(300);
}

void MotorController::RunForDuration(uint16_t motorDuration) {
  if (motorDuration > 0 && xTimerChangePeriod(shortVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(shortVib, 0) == pdPASS) {
    if (pwmValue == 0) {
      SetMotorStrength(255);
    }
    // nrf_gpio_pin_clear(PinMap::Motor);
    nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0); // Restart the PWM sequence with the updated value
  }
}

void MotorController::StartRinging() {
  SetMotorStrength(100);
  RunForDuration(50);
  xTimerStart(longVib, 0);
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_STOP); // Stop the PWM sequence
  pwmValue = 0;                                      // Reset the PWM value
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StartAlarm() {
  SetMotorStrength(80);
  RunForDuration(300);
  xTimerStart(alarmVib, 0);
}

void MotorController::StopAlarm() {
  xTimerStop(alarmVib, 0);
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_STOP); // Stop the PWM sequence
  pwmValue = 0;                                      // Reset the PWM value
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StartWakeAlarm() {
  wakeAlarmStrength = 80;
  wakeAlarmDuration = 100;
  SetMotorStrength(wakeAlarmStrength);
  RunForDuration(wakeAlarmDuration);
  xTimerStart(wakeAlarmVib, 0);
}

void MotorController::WakeAlarmRing(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  if (motorController->wakeAlarmStrength > 40) {
    motorController->wakeAlarmStrength -= 1;
  }
  if (motorController->wakeAlarmDuration < 500) {
    motorController->wakeAlarmDuration += 6;
  }
  motorController->SetMotorStrength(motorController->wakeAlarmStrength);
  motorController->RunForDuration(motorController->wakeAlarmDuration);
}

void MotorController::StopWakeAlarm() {
  xTimerStop(wakeAlarmVib, 0);
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_STOP); // Stop the PWM sequence
  pwmValue = 0;                                      // Reset the PWM value
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StopMotor(TimerHandle_t /*xTimer*/) {
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_STOP); // Stop the PWM sequence
  pwmValue = 0;                                      // Reset the PWM value
  nrf_gpio_pin_set(PinMap::Motor);                   // Set the motor pin to the off state
}

void MotorController::GradualWakeBuzz() {
  SetMotorStrength(60);
  RunForDuration(100);
  // xTimerStart(gradualWakeBuzzDelay, 0);
  // xTimerStart(gradualWakeBuzzEnd, 0);
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
  // xTimerStop(gradualWakeBuzzDelay, 0);
  xTimerStop(gradualWakeBuzzEnd, 0);
  // StopMotor(nullptr);
}
