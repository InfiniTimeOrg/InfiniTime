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

      void SetOffset(uint16_t offset);
      void Reset();

    private:
      std::array<int8_t, 200> data;
      size_t dataIndex = 0;
      float offset;
      Biquad hpf;
      Ptagc agc;
      Biquad lpf;

      int ProcessHeartRate();
    };
  }
}
