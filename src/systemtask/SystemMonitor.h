#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <nrf_log.h>
#include <lvgl/lvgl.h>
#include <malloc.h>

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

          lv_mem_monitor_t mon;
          lv_mem_monitor(&mon);
          NRF_LOG_INFO("#LVGL Memory#\n"
                       " used# %d (%d%%) / free %d\n"
                       " max used# %lu\n"
                       " frag# %d%%\n"
                       " free# %d",
                       static_cast<int>(mon.total_size - mon.free_size),
                       mon.used_pct,
                       mon.free_size,
                       mon.max_used,
                       mon.frag_pct,
                       static_cast<int>(mon.free_biggest_size));

          auto m = mallinfo();
          NRF_LOG_INFO("heap : %d", m.uordblks);

          lastTick = xTaskGetTickCount();
        }
      }

    private:
      mutable TickType_t lastTick = 0;
      mutable TaskStatus_t tasksStatus[10];
    };
  }
}