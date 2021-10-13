#include "logging/NrfLogger.h"

#include <libraries/log/nrf_log.h>
#include <libraries/log/nrf_log_ctrl.h>
#include <libraries/log/nrf_log_default_backends.h>

using namespace Pinetime::Logging;

void NrfLogger::Init() {
  auto result = NRF_LOG_INIT(nullptr);
  APP_ERROR_CHECK(result);

  NRF_LOG_DEFAULT_BACKENDS_INIT();

  if (pdPASS != xTaskCreate(NrfLogger::Process, "LOGGER", 200, this, 0, &m_logger_thread)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void NrfLogger::Process(void*) {
  NRF_LOG_INFO("Logger task started!");
// Suppress endless loop diagnostic
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
  while (true) {
    NRF_LOG_FLUSH();
    vTaskDelay(100); // Not good for power consumption, it will wake up every 100ms...
  }
// Clear diagnostic suppression
#pragma clang diagnostic pop
}

void NrfLogger::Resume() {
  vTaskResume(m_logger_thread);
}
