#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <nrf_log.h>

namespace Pinetime {
  namespace System {
    struct DummyMonitor {};
    struct FreeRtosMonitor {};

    template <class T> class SystemMonitor {
    public:
      SystemMonitor() = delete;
    };

    template <> class SystemMonitor<DummyMonitor> {
    public:
      void Process() const {
      }
    };

    template <> class SystemMonitor<FreeRtosMonitor> {
    public:
      void Process() const {
        if (xTaskGetTickCount() - lastTick > 10000) {
          NRF_LOG_INFO("---------------------------------------\nFree heap : %d", xPortGetFreeHeapSize());
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

    private:
      mutable TickType_t lastTick = 0;
      mutable TaskStatus_t tasksStatus[10];
    };
  }
}