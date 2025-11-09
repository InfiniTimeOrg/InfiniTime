#include <complex>
#include <array>
#include <bit>
#include <numbers>

namespace Pinetime {
  namespace Utility {
    // Fast Fourier transform
    // Implements in-place N to N point complex-to-complex FFT
    // Implements in-place 2N to N point real-to-complex FFT
    // Performing these transforms requires some "twiddling" constants to be known
    // These constants depend only on the size of the transform
    // Since they are expensive to compute, they can only be computed at compile time (consteval)
    class FFT {
    public:
      static consteval std::size_t IntegerLog2(std::size_t n) {
        return std::bit_width(n) - 1;
      }

      template <std::size_t N>
      static consteval std::array<std::complex<float>, IntegerLog2(N)> GenComplexTwiddle() {
        using namespace std::complex_literals;

        std::array<std::complex<float>, IntegerLog2(N)> result;
        for (std::size_t i = 0; i < IntegerLog2(N); i++) {
          result[i] = exp_consteval(-2.i * std::numbers::pi / static_cast<double>(1 << (i + 1)));
        }
        return result;
      }

      template <std::size_t N>
      static consteval std::array<std::complex<float>, (N / 4) - 1> GenRealTwiddle() {
        using namespace std::complex_literals;

        std::array<std::complex<float>, (N / 4) - 1> result;
        for (std::size_t i = 0; i < (N / 4) - 1; i++) {
          result[i] = exp_consteval(-2.i * std::numbers::pi * static_cast<double>(i + 1) / static_cast<double>(N));
        }
        return result;
      }

      template <std::size_t N>
      static void ComplexFFT(std::array<std::complex<float>, N>& array, const std::array<std::complex<float>, IntegerLog2(N)>& twiddle) {
        // In-place Cooley-Tukey
        InplaceBitReverse(array);
        for (std::size_t s = 1; s < IntegerLog2(N) + 1; s++) {
          std::size_t m = 1 << s;
          for (std::size_t k = 0; k < N; k += m) {
            std::complex<float> omega = 1.f;
            for (std::size_t j = 0; j < m / 2; j++) {
              std::complex<float> t = omega * array[k + j + (m / 2)];
              std::complex<float> u = array[k + j];
              array[k + j] = u + t;
              array[k + j + (m / 2)] = u - t;
              omega *= twiddle[s - 1];
            }
          }
        }
      }

      template <std::size_t N>
      static void RealFFT(std::array<std::complex<float>, N>& array,
                          const std::array<std::complex<float>, (N / 2) - 1>& realTwiddle,
                          const std::array<std::complex<float>, IntegerLog2(N)>& complexTwiddle) {
        using namespace std::complex_literals;

        // See https://www.robinscheibler.org/2013/02/13/real-fft.html for how this works
        FFT::ComplexFFT(array, complexTwiddle);
        // Compute DC bin directly (xe/xo simplify)
        // Nyquist bin ignored as unneeded
        array[0] = array[0].real() + array[0].imag();
        // Since computations depend on the inverse of the index (mirrored)
        // compute two at once, outside to inside
        for (std::size_t index = 1; index < N / 2; index++) {
          std::size_t indexInv = N - index;
          std::complex<float> xeLo = (array[index] + std::conj(array[indexInv])) / 2.f;
          std::complex<float> xoLo = -1.if * ((array[index] - std::conj(array[indexInv])) / 2.f);
          std::complex<float> xeHi = (array[indexInv] + std::conj(array[index])) / 2.f;
          std::complex<float> xoHi = -1.if * ((array[indexInv] - std::conj(array[index])) / 2.f);
          array[index] = xeLo + (xoLo * realTwiddle[index - 1]);
          array[indexInv] = xeHi + (xoHi * -std::conj(realTwiddle[index - 1]));
        }
        // Middle element not computed by above loop
        // Since index == indexInv
        // the middle simplifies to the conjugate as the twiddle is always -i
        std::size_t middle = N / 2;
        array[middle] = std::conj(array[middle]);
      }

    private:
      // consteval wrappers of builtins
      template <typename _Tp>
      static consteval std::complex<_Tp> exp_consteval(const std::complex<_Tp>& __z) {
        return polar_consteval<_Tp>(__builtin_exp(__z.real()), __z.imag());
      }

      template <typename _Tp>
      static consteval std::complex<_Tp> polar_consteval(const _Tp& __rho, const _Tp& __theta) {
        return std::complex<_Tp>(__rho * __builtin_cos(__theta), __rho * __builtin_sin(__theta));
      }

      template <class T, std::size_t N>
      static void InplaceBitReverse(std::array<T, N>& array) {
        // Gold-Rader algorithm
        // Faster algorithms exist, but this is sufficient
        std::size_t swapTarget = 0;
        for (std::size_t index = 0; index < N - 1; index++) {
          // Only swap in one direction
          // Otherwise entire array gets swapped twice
          if (index < swapTarget) {
            T temp = array[index];
            array[index] = array[swapTarget];
            array[swapTarget] = temp;
          }
          std::size_t kFactor = N / 2;
          while (kFactor <= swapTarget) {
            swapTarget -= kFactor;
            kFactor /= 2;
          }
          swapTarget += kFactor;
        }
      }
    };
  }
}
