#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include "components/heartrate/Biquad.h"
#include "components/heartrate/Ptagc.h"

namespace Pinetime {
  namespace Controllers {
    class Ppg {
    public:
      static const uint8_t DATA_SIZE = 200;
      Ppg();
      int8_t Preprocess(float spl);
      float HeartRate();

      void SetOffset(uint16_t i);
      void Reset();
      int Compare(int8_t* d1, int shift, size_t count);
      int CompareShift(int8_t* d, int shift, size_t count);
      int Trough(int8_t* d, size_t size, uint8_t mn, uint8_t mx);
      int getRingIndex(int8_t index);

    private:
      std::array<int8_t, DATA_SIZE> data;
      size_t dataIndex = 0;
      float offset;
      Biquad hpf;
      Ptagc agc;
      Biquad lpf;

      float ProcessHeartRate();
    };
  }
}
