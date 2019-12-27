#pragma once
#include <drivers/include/nrfx_saadc.h>


namespace Pinetime {
  namespace Controllers {
    class Battery {
      public:
        void Init();
        void Update();
        float PercentRemaining() const { return percentRemaining; }
        float Voltage() const { return voltage; }
        bool IsCharging() const { return isCharging; }
        bool IsPowerPresent() const { return isPowerPresent; }

      private:
        static void SaadcEventHandler(nrfx_saadc_evt_t const * p_event);
        float percentRemaining = 0.0f;
        float voltage = 0.0f;
        bool isCharging = false;
        bool isPowerPresent = false;
    };
  }
}