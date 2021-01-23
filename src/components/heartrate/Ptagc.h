#pragma once

namespace Pinetime {
  namespace Controllers {
    class Ptagc {
    public:
      Ptagc(float start, float decay, float threshold);
      float Step(float spl);

    private:
      float peak;
      float decay;
      float boost;
      float threshold;

    };
  }
}
