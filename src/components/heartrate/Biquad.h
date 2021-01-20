#pragma once

namespace Pinetime {
  namespace Controllers {
    /// Direct Form II Biquad Filter
    class Biquad {
    public:
      Biquad(float b0, float  b1, float b2, float a1, float a2);
      float Step(float x);

    private:
      float b0;
      float b1;
      float b2;
      float a1;
      float a2;

      float v1 = 0.0f;
      float v2 = 0.0f;
    };
  }
}
