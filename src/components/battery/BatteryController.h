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

      uint8_t PercentRemaining() const {
        return percentRemaining;
      }

      uint16_t Voltage() const {
        return voltage;
      }

      bool IsCharging() const {
        return isCharging;
      }

      bool IsPowerPresent() const {
        return isPowerPresent;
      }

    private:
      static Battery* instance;
      nrf_saadc_value_t saadc_value;

      static constexpr uint32_t chargingPin = 12;
      static constexpr uint32_t powerPresentPin = 19;
      static constexpr nrf_saadc_input_t batteryVoltageAdcInput = NRF_SAADC_INPUT_AIN7;
      uint16_t voltage = 0;
      uint8_t percentRemaining = 0;

      bool isCharging = false;
      bool isPowerPresent = false;

      void SaadcInit();

      void SaadcEventHandler(nrfx_saadc_evt_t const* p_event);
      static void AdcCallbackStatic(nrfx_saadc_evt_t const* event);

      bool isReading = false;
    };
  }
}
