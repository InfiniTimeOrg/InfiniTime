#include <array>

namespace Pinetime {
  namespace Utility {
    struct SOSCoeffs {
      float b0;
      float b1;
      float b2;
      float a1;
      float a2;
    };

    struct FilterState {
      float s1;
      float s2;
    };

    // Infinite impulse response digital filter
    // Implemented as cascaded second order sections (SOS)
    template <std::size_t NSections>
    class IIRFilter {
    public:
      IIRFilter(const std::array<SOSCoeffs, NSections>& coeffs, const std::array<FilterState, NSections>& zi) : coeffs {coeffs}, zi {zi} {};

      float FilterStep(float input) {
        for (std::size_t i = 0; i < NSections; i++) {
          // Transposed form II
          float output = (coeffs[i].b0 * input) + filterStates[i].s1;
          filterStates[i].s1 = ((-coeffs[i].a1) * output) + (coeffs[i].b1 * input) + filterStates[i].s2;
          filterStates[i].s2 = ((-coeffs[i].a2) * output) + (coeffs[i].b2 * input);
          input = output;
        }
        return input;
      }

      void Prime(float value) {
        for (std::size_t i = 0; i < NSections; i++) {
          filterStates[i].s1 = zi[i].s1 * value;
          filterStates[i].s2 = zi[i].s2 * value;
        }
      }

    private:
      std::array<FilterState, NSections> filterStates;
      const std::array<SOSCoeffs, NSections>& coeffs;
      const std::array<FilterState, NSections>& zi;
    };
  }
}
