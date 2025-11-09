#pragma once

#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "components/heartrate/FFT.h"
#include "components/heartrate/IIR.h"
#include "components/heartrate/RLS.h"

namespace Pinetime {
  namespace Controllers {
    class Ppg {
    public:
      Ppg();
      void Ingest(uint16_t hrs, int16_t accX, int16_t accY, int16_t accZ);
      std::optional<uint8_t> HeartRate();
      void Reset();
      [[nodiscard]] bool SufficientData() const;
      static constexpr float sampleDuration = 0.048f;

    private:
      static constexpr float sampleRate = 1.f / sampleDuration;
      // FFT length
      static constexpr uint16_t fftLength = 256;
      // Data going into FFT length (FFT is zero padded)
      static constexpr uint16_t inputLength = std::round(8 * sampleRate);
      // Uupdate rate
      static constexpr float overlapTime = 0.5f;
      // Number of samples before each analysis
      static constexpr uint16_t overlapWindow = std::round(sampleRate * overlapTime);
      // Frequency resolution (Hz)
      static constexpr float binWidth = 1.f / (fftLength * sampleDuration);

      // Input filtering coefficients
      // Generated with scipy butter(4, 0.5, btype="highpass", output="sos", fs=sampleRate)
      static constexpr std::array<Utility::SOSCoeffs, 2> inputCoeffs {
        Utility::SOSCoeffs {.b0 = 0.8209900772315549,
                            .b1 = -1.6419801544631099,
                            .b2 = 0.8209900772315549,
                            .a1 = -1.7363191518098462,
                            .a2 = 0.7562495196628967},
        Utility::SOSCoeffs {.b0 = 1., .b1 = -2., .b2 = 1., .a1 = -1.8698102590459458, .a2 = 0.89127290676215}};
      // Input filtering initial state
      static constexpr std::array<Utility::FilterState, 2> inputInitialState {Utility::FilterState {.s1 = -0.82099008, .s2 = 0.82099008},
                                                                              Utility::FilterState {.s1 = 0., .s2 = 0.}};

      // Adaptive filtering strength
      static constexpr float adaptiveMu = 0.99f;
      // Number of noise channels
      static constexpr size_t noiseChannels = 3;
      // Adaptive filter length
      static constexpr size_t adaptiveLength = std::round(0.25f * sampleRate) * noiseChannels;
      // Adaptive filter reset consideration window
      static constexpr size_t adaptiveResetWindow = 1.f * sampleRate;
      // Adaptive filter reset threshold
      static constexpr float adaptiveResetThresh = 2.f;

      // FFT constants (computed at compile time to avoid expensive runtime calculations)
      static constexpr auto complexTwiddle = Utility::FFT::GenComplexTwiddle<fftLength / 2>();
      static constexpr auto realTwiddle = Utility::FFT::GenRealTwiddle<fftLength>();

      // Prominence filter constants
      // Generated with scipy butter(2, (0.07, 0.65), btype="bandpass", output="sos")
      static constexpr std::array<Utility::SOSCoeffs, 2> prominenceCoeffs {
        Utility::SOSCoeffs {.b0 = 0.3705549357629432,
                            .b1 = 0.7411098715258864,
                            .b2 = 0.3705549357629432,
                            .a1 = 0.5064122272787734,
                            .a2 = 0.2534300618617984},
        Utility::SOSCoeffs {.b0 = 1., .b1 = -2., .b2 = 1., .a1 = -1.6907167610529146, .a2 = 0.7379546730609459}};
      // Prominence filtering initial state
      static constexpr std::array<Utility::FilterState, 2> prominenceInitialState {
        Utility::FilterState {.s1 = 0.47169084512212833, .s2 = 0.15710453541040081},
        Utility::FilterState {.s1 = -0.8422457808850721, .s2 = 0.8422457808850718}};
      static constexpr float primingFactor = 2.f;

      // Threshold after bandpass filtering at which signal is allowed through
      static constexpr float minProminence = 0.5f;
      // Threshold after bandpass filtering at which all signal is allowed through
      static constexpr float maxProminence = 0.8f;
      // Clip values below floor to prevent huge negative logs
      static constexpr float logFloor = 1e-4f;

      // First harmonic minimum relative magnitude to be considered
      static constexpr float f1MagMinimum = 0.5f;
      // First harmonic phase penalty exponent
      static constexpr float f1ExpAttenuationFactor = 4.f;
      // First harmonic magnitude high threshold
      static constexpr float f1MagHigh = 1.f;
      // Second harmonic magnitude high threshold
      static constexpr float f2MagHigh = 0.75f;
      // First harmonic absent energy penality
      static constexpr float noF1Penalty = 0.2f;
      // First harmonic absent energy limit
      static constexpr float noF1EnergyCeiling = 6.f;
      // Harmonic magnitude high energy ceiling
      static constexpr float fnMagHighEnergyCeiling = 3.f;

      // Minimum frequency to consider
      static constexpr float minFrequency = 30.f / 60.f;
      // Maximum frequency to consider
      // Cannot go higher as sample rate puts 2nd harmonic out of range
      static constexpr float maxFrequency = 207.f / 60.f;
      // Minimum frequency FFT bin
      static constexpr size_t minFrequencyBin = (minFrequency / binWidth) + 1;
      // Maximum frequency FFT bin
      static constexpr size_t maxFrequencyBin = (maxFrequency / binWidth) + 1;
      // Length of the region of interest
      static constexpr size_t roiLength = maxFrequencyBin - minFrequencyBin;

      // Minimum log energy to consider a peak strong
      static constexpr float strongPeakEnergyMinimum = 3.f;
      // Minimum prominence of a strong peak to be accepted
      static constexpr float strongPeakProminenceMinimum = 0.8f;
      // Minimum log energy to keep tracking a peak
      static constexpr float trackingEnergyMinimum = -1.f;
      // Minimum log energy to update the HR prediction
      static constexpr float hrUpdateEnergyMinimum = 0.f;
      // Exponential moving average alpha factor for rolling energy
      static constexpr float rollingEnergyAlpha = 0.1f * overlapTime;
      // Exponential moving average alpha factor for heart rate prediction
      static constexpr float hrMovementAlpha = 0.25f * overlapTime;
      // Minimum rolling energy to return the prediction
      static constexpr float rollingEnergyDisplayMinimum = 0.f;
      // Maximum energy at which new strong peaks will replace the current peak
      static constexpr float rollingEnergyWeak = 1.f;

      // Window size for pre-FFT segment RMS normalisation
      static constexpr size_t segmentNormWindow = std::round((1.f / minFrequency) * sampleRate);

      Utility::IIRFilter<inputCoeffs.size()> ppgFilter = Utility::IIRFilter(inputCoeffs, inputInitialState);
      Utility::IIRFilter<inputCoeffs.size()> accXFilter = Utility::IIRFilter(inputCoeffs, inputInitialState);
      Utility::IIRFilter<inputCoeffs.size()> accYFilter = Utility::IIRFilter(inputCoeffs, inputInitialState);
      Utility::IIRFilter<inputCoeffs.size()> accZFilter = Utility::IIRFilter(inputCoeffs, inputInitialState);
      std::array<float, inputLength> adaptiveHrsArray;
      std::array<float, adaptiveResetWindow> filteredHrsArray;
      uint8_t hrsCount;
      uint8_t filteredHrsTailIndex;
      Utility::RLS<adaptiveLength, noiseChannels> accAdaptive = Utility::RLS<adaptiveLength, noiseChannels>(adaptiveMu);

      std::array<std::complex<float>, fftLength / 2> complexFftArray;
      std::span<float, fftLength> fftArray {reinterpret_cast<float*>(complexFftArray.data()), fftLength};

      Utility::IIRFilter<prominenceCoeffs.size()> prominenceFilter = Utility::IIRFilter(prominenceCoeffs, prominenceInitialState);

      std::optional<float> lockedHrBin;
      float rollingEnergy;
      bool ready;

      void StoreHrs(float filteredHrs);
      void RunAlg();
      void ProminenceFilter();
      void SegmentRMSNorm();
      void HarmonicFilter();
    };
  }
}
