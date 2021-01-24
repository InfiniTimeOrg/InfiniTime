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
  template <int N> 
  class CircBuffer {
  public:
    CircBuffer() : arr{}, sz{}, cap{N}, head{} {}
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
    uint8_t sz; /**< The current size of the array.*/
    uint8_t cap; /**< Total capacity of the CircBuffer.*/
    uint8_t head; /**< The current head of the CircBuffer*/
  };

    class Battery {
      public:
        void Init();
        void Update();
        int PercentRemaining() const { return percentRemainingBuffer.GetAverage(); }
        float Voltage() const { return voltage; }
        bool IsCharging() const { return isCharging; }
        bool IsPowerPresent() const { return isPowerPresent; }

      private:
        static constexpr uint32_t chargingPin = 12;
        static constexpr uint32_t powerPresentPin = 19;
        static constexpr nrf_saadc_input_t batteryVoltageAdcInput = NRF_SAADC_INPUT_AIN7;
        static constexpr uint8_t percentRemainingSamples = 10;
        static void SaadcEventHandler(nrfx_saadc_evt_t const * p_event);
        CircBuffer<percentRemainingSamples> percentRemainingBuffer {};
        float voltage = 0.0f;
        bool isCharging = false;
        bool isPowerPresent = false;
    };
  }
}