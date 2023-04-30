#include "components/heartrate/Ppg.h"
#include <nrf_log.h>
#include <vector>

using namespace Pinetime::Controllers;

namespace {
  float LinearInterpolation(const float* xValues, const float* yValues, int length, float pointX) {
    if (pointX > xValues[length - 1]) {
      return yValues[length - 1];
    } else if (pointX <= xValues[0]) {
      return yValues[0];
    }
    int index = 0;
    while (pointX > xValues[index] && index < length - 1) {
      index++;
    }
    float pointX0 = xValues[index - 1];
    float pointX1 = xValues[index];
    float pointY0 = yValues[index - 1];
    float pointY1 = yValues[index];
    float mu = (pointX - pointX0) / (pointX1 - pointX0);

    return (pointY0 * (1 - mu) + pointY1 * mu);
  }

  float PeakSearch(float* xVals, float* yVals, float threshold, float& width, float start, float end, int length) {
    int peaks = 0;
    bool enabled = false;
    float minBin = 0.0f;
    float maxBin = 0.0f;
    float peakCenter = 0.0f;
    float prevValue = LinearInterpolation(xVals, yVals, length, start - 0.01f);
    float currValue = LinearInterpolation(xVals, yVals, length, start);
    float idx = start;
    while (idx < end) {
      float nextValue = LinearInterpolation(xVals, yVals, length, idx + 0.01f);
      if (currValue < threshold) {
        enabled = true;
      }
      if (currValue >= threshold and enabled) {
        if (prevValue < threshold) {
          minBin = idx;
        } else if (nextValue <= threshold) {
          maxBin = idx;
          peaks++;
          width = maxBin - minBin;
          peakCenter = width / 2.0f + minBin;
        }
      }
      prevValue = currValue;
      currValue = nextValue;
      idx += 0.01f;
    }
    if (peaks != 1) {
      width = 0.0f;
      peakCenter = 0.0f;
    }
    return peakCenter;
  }

  float SpectrumMean(const std::array<float, Ppg::spectrumLength>& signal, int start, int end) {
    int total = 0;
    float mean = 0.0f;
    for (int idx = start; idx < end; idx++) {
      mean += signal.at(idx);
      total++;
    }
    if (total > 0) {
      mean /= static_cast<float>(total);
    }
    return mean;
  }

  float SignalToNoise(const std::array<float, Ppg::spectrumLength>& signal, int start, int end, float max) {
    float mean = SpectrumMean(signal, start, end);
    return max / mean;
  }

  // Simple bandpass filter using exponential moving average
  void Filter30to240(std::array<float, Ppg::dataLength>& signal) {
    // From:
    // https://www.norwegiancreations.com/2016/03/arduino-tutorial-simple-high-pass-band-pass-and-band-stop-filtering/

    int length = signal.size();
    // 0.268 is ~0.5Hz and 0.816 is ~4Hz cutoff at 10Hz sampling
    float expAlpha = 0.816f;
    float expAvg = 0.0f;
    for (int loop = 0; loop < 4; loop++) {
      expAvg = signal.front();
      for (int idx = 0; idx < length; idx++) {
        expAvg = (expAlpha * signal.at(idx)) + ((1 - expAlpha) * expAvg);
        signal[idx] = expAvg;
      }
    }
    expAlpha = 0.268f;
    for (int loop = 0; loop < 4; loop++) {
      expAvg = signal.front();
      for (int idx = 0; idx < length; idx++) {
        expAvg = (expAlpha * signal.at(idx)) + ((1 - expAlpha) * expAvg);
        signal[idx] -= expAvg;
      }
    }
  }

  float SpectrumMax(const std::array<float, Ppg::spectrumLength>& data, int start, int end) {
    float max = 0.0f;
    for (int idx = start; idx < end; idx++) {
      if (data.at(idx) > max) {
        max = data.at(idx);
      }
    }
    return max;
  }

  void Detrend(std::array<float, Ppg::dataLength>& signal) {
    int size = signal.size();
    float offset = signal.front();
    float slope = (signal.at(size - 1) - offset) / static_cast<float>(size - 1);

    for (int idx = 0; idx < size; idx++) {
      signal[idx] -= (slope * static_cast<float>(idx) + offset);
    }
    for (int idx = 0; idx < size - 1; idx++) {
      signal[idx] = signal[idx + 1] - signal[idx];
    }
  }

  // Hanning Coefficients from numpy: python -c 'import numpy;print(numpy.hanning(64))'
  // Note: Harcoded and must be updated if constexpr dataLength is changed. Prevents the need to
  // use cosf() which results in an extra ~5KB in storage.
  // This data is symetrical so just using the first half (saves 128B when dataLength is 64).
  static constexpr float hanning[Ppg::dataLength >> 1] {
    0.0f,        0.00248461f, 0.00991376f, 0.0222136f,  0.03926189f, 0.06088921f, 0.08688061f, 0.11697778f,
    0.15088159f, 0.1882551f,  0.22872687f, 0.27189467f, 0.31732949f, 0.36457977f, 0.41317591f, 0.46263495f,
    0.51246535f, 0.56217185f, 0.61126047f, 0.65924333f, 0.70564355f, 0.75f,       0.79187184f, 0.83084292f,
    0.86652594f, 0.89856625f, 0.92664544f, 0.95048443f, 0.96984631f, 0.98453864f, 0.99441541f, 0.99937846f};
}

Ppg::Ppg() {
  dataAverage.fill(0.0f);
  spectrum.fill(0.0f);
}

int8_t Ppg::Preprocess(uint32_t hrs, uint32_t als) {
  if (dataIndex < dataLength) {
    dataHRS[dataIndex++] = hrs;
  }
  alsValue = als;
  if (alsValue > alsThreshold) {
    return 1;
  }
  return 0;
}

int Ppg::HeartRate() {
  if (dataIndex < dataLength) {
    return 0;
  }
  int hr = 0;
  hr = ProcessHeartRate(resetSpectralAvg);
  resetSpectralAvg = false;
  // Make room for overlapWindow number of new samples
  for (int idx = 0; idx < dataLength - overlapWindow; idx++) {
    dataHRS[idx] = dataHRS[idx + overlapWindow];
  }
  dataIndex = dataLength - overlapWindow;
  return hr;
}

void Ppg::Reset(bool resetDaqBuffer) {
  if (resetDaqBuffer) {
    dataIndex = 0;
  }
  avgIndex = 0;
  dataAverage.fill(0.0f);
  lastPeakLocation = 0.0f;
  alsThreshold = UINT16_MAX;
  alsValue = 0;
  resetSpectralAvg = true;
  spectrum.fill(0.0f);
}

// Pass init == true to reset spectral averaging.
// Returns -1 (Reset Acquisition), 0 (Unable to obtain HR) or HR (BPM).
int Ppg::ProcessHeartRate(bool init) {
  std::copy(dataHRS.begin(), dataHRS.end(), vReal.begin());
  Detrend(vReal);
  Filter30to240(vReal);
  vImag.fill(0.0f);
  // Apply Hanning Window
  int hannIdx = 0;
  for (int idx = 0; idx < dataLength; idx++) {
    if (idx >= dataLength >> 1) {
      hannIdx--;
    }
    vReal[idx] *= hanning[hannIdx];
    if (idx < dataLength >> 1) {
      hannIdx++;
    }
  }
  // Compute in place power spectrum
  ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal.data(), vImag.data(), dataLength, sampleFreq);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();
  FFT.~ArduinoFFT();
  SpectrumAverage(vReal.data(), spectrum.data(), spectrum.size(), init);
  peakLocation = 0.0f;
  float threshold = peakDetectionThreshold;
  float peakWidth = 0.0f;
  int specLen = spectrum.size();
  float max = SpectrumMax(spectrum, hrROIbegin, hrROIend);
  float signalToNoiseRatio = SignalToNoise(spectrum, hrROIbegin, hrROIend, max);
  if (signalToNoiseRatio > signalToNoiseThreshold && spectrum.at(0) < dcThreshold) {
    threshold *= max;
    // Reuse VImag for interpolation x values passed to PeakSearch
    for (int idx = 0; idx < dataLength; idx++) {
      vImag[idx] = idx;
    }
    peakLocation = PeakSearch(vImag.data(),
                              spectrum.data(),
                              threshold,
                              peakWidth,
                              static_cast<float>(hrROIbegin),
                              static_cast<float>(hrROIend),
                              specLen);
    peakLocation *= freqResolution;
  }
  // Peak too wide? (broad spectrum noise or large, rapid HR change)
  if (peakWidth > maxPeakWidth) {
    peakLocation = 0.0f;
  }
  // Check HR limits
  if (peakLocation < minHR || peakLocation > maxHR) {
    peakLocation = 0.0f;
  }
  // Reset spectral averaging if bad reading
  if (peakLocation == 0.0f) {
    resetSpectralAvg = true;
  }
  // Set the ambient light threshold and return HR in BPM
  alsThreshold = static_cast<uint16_t>(alsValue * alsFactor);
  // Get current average HR. If HR reduced to zero, return -1 (reset) else HR
  peakLocation = HeartRateAverage(peakLocation);
  int rtn = -1;
  if (peakLocation == 0.0f && lastPeakLocation > 0.0f) {
    lastPeakLocation = 0.0f;
  } else {
    lastPeakLocation = peakLocation;
    rtn = static_cast<int>((peakLocation * 60.0f) + 0.5f);
  }
  return rtn;
}

void Ppg::SpectrumAverage(const float* data, float* spectrum, int length, bool reset) {
  if (reset) {
    spectralAvgCount = 0;
  }
  float count = static_cast<float>(spectralAvgCount);
  for (int idx = 0; idx < length; idx++) {
    spectrum[idx] = (spectrum[idx] * count + data[idx]) / (count + 1);
  }
  if (spectralAvgCount < spectralAvgMax) {
    spectralAvgCount++;
  }
}

float Ppg::HeartRateAverage(float hr) {
  avgIndex++;
  avgIndex %= dataAverage.size();
  dataAverage[avgIndex] = hr;
  float avg = 0.0f;
  float total = 0.0f;
  float min = 300.0f;
  float max = 0.0f;
  for (const float& value : dataAverage) {
    if (value > 0.0f) {
      avg += value;
      if (value < min)
        min = value;
      if (value > max)
        max = value;
      total++;
    }
  }
  if (total > 0) {
    avg /= total;
  } else {
    avg = 0.0f;
  }
  return avg;
}
