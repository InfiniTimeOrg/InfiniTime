#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    static constexpr uint8_t pinMotor = 16;

    class MotorController {
    public:
      void Init();
      void SetDuration(uint8_t motorDuration);
      #ifndef NRF_CLOCK_ENABLED 
      #define NRF_CLOCK_ENABLED 1
      #endif

      #ifndef CLOCK_CONFIG_LF_SRC
      #define CLOCK_CONFIG_LF_SRC 1
      #endif

      #ifndef CLOCK_CONFIG_IRQ_PRIORITY
      #define CLOCK_CONFIG_IRQ_PRIORITY 6
      #endif

      #define APP_TIMER_ENABLED 1
      #define APP_TIMER_CONFIG_RTC_FREQUENCY 15 //2048hz
      #define APP_TIMER_CONFIG_IRQ_PRIORITY 6

      #ifndef APP_TIMER_CONFIG_OP_QUEUE_SIZE
      #define APP_TIMER_CONFIG_OP_QUEUE_SIZE 10
      #endif

      #ifndef APP_TIMER_CONFIG_USE_SCHEDULER
      #define APP_TIMER_CONFIG_USE_SCHEDULER 0
      #endif

      #ifndef APP_TIMER_KEEPS_RTC_ACTIVE
      #define APP_TIMER_KEEPS_RTC_ACTIVE 0
      #endif

      #ifndef APP_TIMER_SAFE_WINDOW_MS
      #define APP_TIMER_SAFE_WINDOW_MS 300000
      #endif

      #ifndef APP_TIMER_WITH_PROFILER
      #define APP_TIMER_WITH_PROFILER 0
      #endif

      #ifndef APP_TIMER_CONFIG_SWI_NUMBER
      #define APP_TIMER_CONFIG_SWI_NUMBER 0
      #endif

    private:

    };
  }
}