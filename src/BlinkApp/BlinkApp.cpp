#include "BlinkApp.h"
#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <boards.h>

using namespace Pinetime::Applications;

void BlinkApp::Start() {
  if (pdPASS != xTaskCreate(BlinkApp::Process, "BlinkApp", 256, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void BlinkApp::Process(void *instance) {
  auto* app = static_cast<BlinkApp*>(instance);

  NRF_LOG_INFO("BlinkApp task started!");
  while (1) {
//    NRF_LOG_INFO("BlinkApp task running!");
//    nrf_gpio_pin_toggle(22);
//    nrf_gpio_pin_toggle(23);
//    nrf_gpio_pin_toggle(14);
    vTaskDelay(1000);
  }
}
