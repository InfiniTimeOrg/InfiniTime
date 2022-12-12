#pragma once

#include "libs/arduinoFFT/arduinoFFT.h"
#include "libs/Arduino-Interpolation/InterpolationLib.h"
#include <array>
#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class Ppg {
    public:
      Ppg();
      int8_t Preprocess(uint32_t hrs, uint32_t als);
      int HeartRate();
      void Reset(bool resetDaqBuffer);
      static constexpr int deltaTms = 100;
      // Daq dataLength: Must be power of 2
      static constexpr int dataLength = 64;
      static constexpr int spectrumLength = dataLength >> 1;

    private:
      // The sampling frequency (Hz) based on sampling time in milliseconds (DeltaTms)
      static constexpr float sampleFreq = 1000.0f / static_cast<float>(deltaTms);
      // The frequency resolution (Hz)
      static constexpr float freqResolution = sampleFreq / dataLength;
      // Number of samples before each analysis
      // 0.5 second update rate at 10Hz
      static constexpr int overlapWindow = 5;
      // Maximum number of spectrum running averages
      // Note: actual number of spectra averaged = spectralAvgMax + 1
      static constexpr int spectralAvgMax = 2;
      // Multiple Peaks above this threshold (% of max) are rejected
      static constexpr float peakDetectionThreshold = 0.6f;
      // Maximum peak width (bins) at threshold for valid peak.
      static constexpr float maxPeakWidth = 2.5f;
      // Metric for spectrum noise level.
      static constexpr float signalToNoiseThreshold = 3.0f;
      // Heart rate Region Of Interest begin (bins)
      static constexpr int hrROIbegin = static_cast<int>((30.0f / 60.0f) / freqResolution + 0.5f);
      // Heart rate Region Of Interest end (bins)
      static constexpr int hrROIend = static_cast<int>((240.0f / 60.0f) / freqResolution + 0.5f);
      // Minimum HR (Hz)
      static constexpr float minHR = 40.0f / 60.0f;
      // Maximum HR (Hz)
      static constexpr float maxHR = 230.0f / 60.0f;
      // Threshold for high DC level after filtering
      static constexpr float dcThreshold = 0.5f;
      // ALS detection factor
      static constexpr float alsFactor = 2.0f;

      arduinoFFT FFT;
      // Raw ADC data
      std::array<uint16_t, dataLength> dataHRS;
      // Stores Real numbers from FFT
      std::array<float, dataLength> vReal;
      // Stores Imaginary numbers from FFT
      std::array<float, dataLength> vImag;
      // Stores power spectrum calculated from FFT real and imag values
      std::array<float, (spectrumLength)> spectrum;
      // Stores each new HR value (Hz). Non zero values are averaged for HR output
      std::array<float, 20> dataAverage;

      int avgIndex = 0;
      int spectralAvgCount = 0;
      float lastPeakLocation = 0.0f;
      uint16_t alsThreshold = UINT16_MAX;
      uint16_t alsValue = 0;
      int dataIndex = 0;
      float peakLocation;
      bool resetSpectralAvg = true;

      int ProcessHeartRate(bool init);
      float HeartRateAverage(float hr);
      void SpectrumAverage(const float* data, float* spectrum, int length, bool reset);
    };
  }
}
