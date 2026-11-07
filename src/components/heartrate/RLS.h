#include <array>
#include <cstdlib>
#include <optional>
#include <functional>
#include <numeric>

namespace Pinetime {
  namespace Utility {
    // Recursive least squares adaptive filter
    // Supports N noise channels concurrently (NoiseStep)
    // See https://en.wikipedia.org/wiki/Recursive_least_squares_filter
    // and https://matousc89.github.io/padasip/sources/filters/rls.html
    template <std::size_t FIRLength, std::size_t NoiseStep>
    class RLS {
    public:
      explicit RLS(float mu) {
        this->mu = mu;
        Reset();
      }

      void Reset(bool hard = true) {
        if (hard) {
          startupRemaining = FIRLength;
        }
        for (std::size_t i = 0; i < FIRLength; i++) {
          for (std::size_t j = 0; j < FIRLength; j++) {
            correlationInv[i][j] = 0.f;
          }
          correlationInv[i][i] = 1.f / eps;
          weights[i] = 0.f;
        }
      }

      float FilterStep(float dataPoint, std::array<float, NoiseStep> noisePoints, std::optional<float> resetThresh) {
        for (std::size_t index = 0; index < FIRLength - NoiseStep; index++) {
          noiseHistory[index] = noiseHistory[index + NoiseStep];
        }
        for (std::size_t index = FIRLength - NoiseStep; index < FIRLength; index++) {
          noiseHistory[index] = noisePoints[index - FIRLength + NoiseStep];
        }
        // Don't filter until noise points ready
        if (startupRemaining > 0) {
          startupRemaining -= NoiseStep;
          if (startupRemaining > 0) {
            return dataPoint;
          }
        }

        float noiseEstimate = DotProduct(weights, noiseHistory);
        float denoisedDataPoint = dataPoint - noiseEstimate;

        // Resetting is useful to recover from filter divergence
        // If the denoised output exceeds the chosen threshold, reset the filter state
        if (resetThresh.has_value()) {
          if (std::abs(denoisedDataPoint) > resetThresh) {
            Reset(false);
            denoisedDataPoint = dataPoint;
          }
        }

        std::array<std::array<float, FIRLength>, FIRLength> correlationUpdate;
        for (auto& arr : correlationUpdate) {
          arr.fill(0.f);
        }

        MatrixMulDynamic(
          correlationInv,
          [this](std::size_t x, std::size_t y) {
            return noiseHistory[x] * noiseHistory[y];
          },
          correlationUpdate);

        MatrixMulInplace(correlationUpdate, correlationInv);

        float normFactor = mu + CalculateNormFactor();

        for (std::size_t i = 0; i < FIRLength; i++) {
          for (std::size_t j = 0; j < FIRLength; j++) {
            correlationInv[i][j] = (1.f / mu) * (correlationInv[i][j] - (correlationUpdate[i][j] / normFactor));
          }
        }

        DotMatrixInplaceAdd(correlationInv, noiseHistory, weights, denoisedDataPoint);

        return denoisedDataPoint;
      }

    private:
      // noinline to bound stack use more tightly
      [[gnu::noinline]] float CalculateNormFactor() {
        std::array<float, FIRLength> temp;
        temp.fill(0.f);
        DotMatrixInplaceAdd(correlationInv, noiseHistory, temp);
        return DotProduct(temp, noiseHistory);
      }

      [[gnu::noinline]] static void MatrixMulInplace(std::array<std::array<float, FIRLength>, FIRLength>& a,
                                                     const std::array<std::array<float, FIRLength>, FIRLength>& b) {
        // Uses 1 row of extra space
        for (std::size_t i = 0; i < FIRLength; i++) {
          std::array<float, FIRLength> aPrior {a[i]};
          for (std::size_t j = 0; j < FIRLength; j++) {
            float acc = 0.f;
            for (std::size_t k = 0; k < FIRLength; k++) {
              acc += aPrior[k] * b[k][j];
            }
            a[i][j] = acc;
          }
        }
      }

      static float DotProduct(const std::array<float, FIRLength>& a, const std::array<float, FIRLength>& b) {
        return std::inner_product(a.begin(), a.end(), b.begin(), 0.f);
      }

      static void DotMatrixInplaceAdd(std::array<std::array<float, FIRLength>, FIRLength>& mat,
                                      const std::array<float, FIRLength>& vec,
                                      std::array<float, FIRLength>& out,
                                      float norm = 1.f) {
        for (std::size_t i = 0; i < FIRLength; i++) {
          out[i] += DotProduct(mat[i], vec) * norm;
        }
      }

      static void MatrixMulDynamic(const std::array<std::array<float, FIRLength>, FIRLength>& mat,
                                   const std::function<float(std::size_t x, std::size_t y)>& matGen,
                                   std::array<std::array<float, FIRLength>, FIRLength>& out) {
        for (std::size_t i = 0; i < FIRLength; i++) {
          for (std::size_t j = 0; j < FIRLength; j++) {
            float acc = 0.f;
            for (std::size_t k = 0; k < FIRLength; k++) {
              acc += mat[i][k] * matGen(k, j);
            }
            out[i][j] = acc;
          }
        }
      }

      static constexpr float eps = 1.f;
      std::array<float, FIRLength> weights;
      std::array<float, FIRLength> noiseHistory;
      std::array<std::array<float, FIRLength>, FIRLength> correlationInv;
      float mu;
      std::size_t startupRemaining;
    };
  }
}
