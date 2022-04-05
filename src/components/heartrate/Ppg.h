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
      float HeartRate();

      void SetOffset(uint16_t i);
      void Reset();

    private:
      static const uint8_t DATA_SIZE = 200;
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
