#include "MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"

APP_TIMER_DEF(vibTimer);

using namespace Pinetime::Controllers;

static void lfclk_request(void)     //get the low freq. clock
{
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
}

void vibrateTimer(void * p_context)
{
    nrf_gpio_pin_set(pinMotor);
}

static void create_timers()
{
    //create timer, single shot, re-armable
    app_timer_create(&vibTimer, APP_TIMER_MODE_SINGLE_SHOT, vibrateTimer);
}

void MotorController::Init() {
    nrf_gpio_cfg_output(pinMotor); // set the motor pin as an output 
    nrf_gpio_pin_set(pinMotor);
    lfclk_request();        //get lfclock ready
    app_timer_init();       //start app timers to make calls
    create_timers();
}

void MotorController::SetDuration(uint8_t motorDuration) {    
    nrf_gpio_pin_clear(pinMotor);
    //start timer for motorDuration miliseconds
    app_timer_start(vibTimer, APP_TIMER_TICKS(motorDuration), NULL); //timers trigger at end of duration?
}
