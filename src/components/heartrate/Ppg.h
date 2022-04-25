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
      Ppg();
      int8_t Preprocess(float spl);
      int HeartRate();

      void SetOffset(uint16_t i);
      void Reset();

    private:
      static const uint8_t UPDATE_HEARTRATE_AFTER = 200;
      std::array<int8_t, 200> data;
      size_t dataIndex = 0;
      float offset;
      Biquad hpf;
      Ptagc agc;
      Biquad lpf;

      int getRingIndex(int8_t index);
      int Compare(int8_t* d1, int shift, size_t count);
      int CompareShift(int8_t* d, int shift, size_t count);
      int Trough(int8_t* d, size_t size, uint8_t mn, uint8_t mx);
      int ProcessHeartRate();
    };
  }
}
