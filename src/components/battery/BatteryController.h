#pragma once
#include <cstdint>
#include <drivers/include/nrfx_saadc.h>
#include <array>
#include <numeric>

namespace Pinetime {
  namespace Controllers {

    /** A simple circular buffer that can be used to average
     out the sensor values. The total capacity of the CircBuffer
    is given as the template parameter N.
    */
    template <int N> class CircBuffer {
    public:
      CircBuffer() : arr {}, sz {}, cap {N}, head {} {
      }
      /**
     insert member function overwrites the next data to the current
    HEAD and moves the HEAD to the newly inserted value.
    */
      void insert(const int num) {
        head %= cap;
        arr[head++] = num;
        if (sz != cap) {
          sz++;
        }
      }

      int GetAverage() const {
        int sum = std::accumulate(arr.begin(), arr.end(), 0);
        return (sum / sz);
      }

    private:
      std::array<int, N> arr; /**< internal array used to store the values*/
      uint8_t sz;             /**< The current size of the array.*/
      uint8_t cap;            /**< Total capacity of the CircBuffer.*/
      uint8_t head;           /**< The current head of the CircBuffer*/
    };

    class Battery {
    public:
      Battery();

      void Init();
      void Update();

      int PercentRemaining() const {
        return percentRemainingBuffer.GetAverage();
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

      static constexpr uint8_t percentRemainingSamples = 5;
      CircBuffer<percentRemainingSamples> percentRemainingBuffer {};

      static constexpr uint32_t chargingPin = 12;
      static constexpr uint32_t powerPresentPin = 19;
      static constexpr nrf_saadc_input_t batteryVoltageAdcInput = NRF_SAADC_INPUT_AIN7;
      uint16_t voltage = 0;
      int percentRemaining = -1;

      bool isCharging = false;
      bool isPowerPresent = false;

      void SaadcInit();

      void SaadcEventHandler(nrfx_saadc_evt_t const* p_event);
      static void adcCallbackStatic(nrfx_saadc_evt_t const* event);

      bool isReading = false;
      uint8_t samples = 0;
    };
  }
}
