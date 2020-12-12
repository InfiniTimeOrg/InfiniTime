#pragma once
#include <cstdint>
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
        static constexpr uint32_t chargingPin = 12;
        static constexpr uint32_t powerPresentPin = 19;
        static constexpr nrf_saadc_input_t batteryVoltageAdcInput = NRF_SAADC_INPUT_AIN7;
        static void SaadcEventHandler(nrfx_saadc_evt_t const * p_event);
        float percentRemaining = 0.0f;
        float voltage = 0.0f;
        bool isCharging = false;
        bool isPowerPresent = false;
    };
  }
}