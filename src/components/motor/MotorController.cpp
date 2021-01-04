#include "MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

void MotorController::Init() {
    nrf_gpio_cfg_output(pinMotor); // set the motor pin as an output
    SetDuration(20);    //miliseconds
}

void MotorController::SetDuration(uint8_t motorDuration) {
    nrf_gpio_pin_clear(pinMotor);
    vTaskDelay(motorDuration);
    nrf_gpio_pin_set(pinMotor);
}
