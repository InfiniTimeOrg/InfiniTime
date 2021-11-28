#pragma once

#include <array>
#include "components/heartrate/Biquad.h"
#include "components/heartrate/Ptagc.h"

namespace Pinetime {
  namespace Controllers {
    class Ppg {
    public:
      Ppg();
      int8_t Preprocess(float spl);
      float HeartRate();

      void SetOffset(uint16_t i);
      void Reset();

    private:
      std::array<int8_t, 200> data;
      size_t dataIndex = 0;
      float offset;
      Biquad hpf;
      Ptagc agc;
      Biquad lpf;

      float ProcessHeartRate();
    };
  }
}
