#include <FreeRTOS.h>
#include <task.h>
#include <BlinkApp/BlinkApp.h>
#include <libraries/bsp/bsp.h>
#include <legacy/nrf_drv_clock.h>
#include <libraries/timer/app_timer.h>
#include <libraries/gpiote/app_gpiote.h>
#include <DisplayApp/DisplayApp.h>
#include <softdevice/common/nrf_sdh.h>

#include <softdevice/common/nrf_sdh_freertos.h>

#include "BLE/BleManager.h"

#if NRF_LOG_ENABLED
#include "Logging/NrfLogger.h"
Pinetime::Logging::NrfLogger logger;
#else
#include "Logging/DummyLogger.h"
Pinetime::Logging::DummyLogger logger;
#endif

Pinetime::Applications::BlinkApp blinkApp;
Pinetime::Applications::DisplayApp displayApp;
TaskHandle_t systemThread;

extern "C" {
  void vApplicationIdleHook() {
    logger.Resume();
  }

  void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName ) {
    bsp_board_led_on(3);
  }
}

void SystemTask(void *) {
  APP_GPIOTE_INIT(2);
  app_timer_init();
  bool erase_bonds=false;
  nrf_sdh_freertos_init(ble_manager_start_advertising, &erase_bonds);
//  blinkApp.Start();
  displayApp.Start();

  while (1) {
    vTaskSuspend(nullptr);
  }
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



