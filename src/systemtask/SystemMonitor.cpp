#include "systemtask/SystemTask.h"
#if configUSE_TRACE_FACILITY == 1
  // FreeRtosMonitor
  #include <FreeRTOS.h>
  #include <task.h>
  #include <nrf_log.h>

void Pinetime::System::SystemMonitor::Process() {
  if (xTaskGetTickCount() - lastTick > 10000) {
    NRF_LOG_INFO("---------------------------------------\nFree heap : %d", xPortGetFreeHeapSize());
    TaskStatus_t tasksStatus[10];
    auto nb = uxTaskGetSystemState(tasksStatus, 10, nullptr);
    for (uint32_t i = 0; i < nb; i++) {
      NRF_LOG_INFO("Task [%s] - %d", tasksStatus[i].pcTaskName, tasksStatus[i].usStackHighWaterMark);
      if (tasksStatus[i].usStackHighWaterMark < 20)
        NRF_LOG_INFO("WARNING!!! Task %s task is nearly full, only %dB available",
                     tasksStatus[i].pcTaskName,
                     tasksStatus[i].usStackHighWaterMark * 4);
    }
    lastTick = xTaskGetTickCount();
  }
}
#else
// DummyMonitor
void Pinetime::System::SystemMonitor::Process() {
}
#endif
