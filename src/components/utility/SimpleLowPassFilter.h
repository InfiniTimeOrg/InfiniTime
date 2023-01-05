#pragma once

namespace Pinetime {
  namespace Utility {

    // See https://en.wikipedia.org/wiki/Low-pass_filter
    template <typename Value, Value k = 2> class SimpleLowPassFilter {
    public:
      SimpleLowPassFilter() = default;

      SimpleLowPassFilter(Value start) : prev {start} {
      }

      Value GetValue(Value current) {
        return prev += (current - prev) / k;
      }

      void Reset(Value start) {
        prev = start;
      }

    private:
      Value prev;
    };
  }
}
