#pragma once
#include <cstdint>
#include <drivers/include/nrfx_saadc.h>
#include <array>
#include <numeric>

namespace Pinetime {
  namespace Controllers {

    class Battery {
      public:

        Battery();

        void Init();
        void Update();
        
        int PercentRemaining();
        float Voltage();

        bool IsCharging() const { return isCharging; }
        bool IsPowerPresent() const { return isPowerPresent; }

      private:
        static Battery *instance;
        nrf_saadc_value_t  saadc_value;

        static constexpr uint32_t chargingPin = 12;
        static constexpr uint32_t powerPresentPin = 19;
        static constexpr nrf_saadc_input_t batteryVoltageAdcInput = NRF_SAADC_INPUT_AIN7;
        float voltage = 0.0f;
        int percentRemaining = -1;

        bool isCharging = false;
        bool isPowerPresent = false;
        
        void SaadcInit();

        void SaadcEventHandler(nrfx_saadc_evt_t const * p_event);
        static void adcCallbackStatic(nrfx_saadc_evt_t const *event);
    };
  }
}