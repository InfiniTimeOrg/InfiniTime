#include <FreeRTOS.h>
#include <task.h>
#include <libraries/bsp/bsp.h>
#include <legacy/nrf_drv_clock.h>
#include <libraries/timer/app_timer.h>
#include <libraries/gpiote/app_gpiote.h>
#include <DisplayApp/DisplayApp.h>
#include <softdevice/common/nrf_sdh.h>

#include <softdevice/common/nrf_sdh_freertos.h>
#include <hal/nrf_rtc.h>
#include <timers.h>

#include "BLE/BleManager.h"

#if NRF_LOG_ENABLED
#include "Logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
#include "Logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

Pinetime::Applications::DisplayApp displayApp;
TaskHandle_t systemThread;
bool isSleeping = false;
TimerHandle_t debounceTimer;

extern "C" {
  void vApplicationIdleHook() {
    logger.Resume();
  }

  void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName ) {
    bsp_board_led_on(3);
  }
}



void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTimerStartFromISR(debounceTimer, &xHigherPriorityTaskWoken);
  // TODO should I do something if xHigherPriorityTaskWoken == pdTRUE?
}

void DebounceTimerCallback(TimerHandle_t xTimer) {
  xTimerStop(xTimer, 0);
  if(isSleeping) {
    displayApp.PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToRunning);
    isSleeping = false;
  }
  else {
    displayApp.PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToSleep);
    isSleeping = true;
  }
}

void SystemTask(void *) {
  APP_GPIOTE_INIT(2);
  bool erase_bonds=false;
  nrf_sdh_freertos_init(ble_manager_start_advertising, &erase_bonds);
  displayApp.Start();

  debounceTimer = xTimerCreate ("debounceTimer", 200, pdFALSE, (void *) 0, DebounceTimerCallback);

  nrf_gpio_cfg_sense_input(13, (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pulldown, (nrf_gpio_pin_sense_t)GPIO_PIN_CNF_SENSE_High);
  nrf_gpio_cfg_output(15);
  nrf_gpio_pin_set(15);

  nrfx_gpiote_in_config_t pinConfig;
  pinConfig.skip_gpio_setup = true;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;
  pinConfig.sense = (nrf_gpiote_polarity_t)NRF_GPIOTE_POLARITY_HITOLO;
  pinConfig.pull = (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pulldown;

  nrfx_gpiote_in_init(13, &pinConfig, nrfx_gpiote_evt_handler);

  vTaskSuspend(nullptr);
}

void OnNewTime(uint8_t minutes, uint8_t hours) {
  displayApp.SetTime(minutes, hours);
}

int main(void) {
  logger.Init();
  nrf_drv_clock_init();

  if (pdPASS != xTaskCreate(SystemTask, "MAIN", 256, nullptr, 0, &systemThread))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);

  ble_manager_init();
  ble_manager_set_callback(OnNewTime);

  vTaskStartScheduler();

  for (;;) {
    APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
  }
}



