#include "components/heartrate/Ppg.h"
#include <nrf_log.h>

using namespace Pinetime::Controllers;

namespace {
  // Computes a mod b
  // fmod is a remainder rather than a true mathematical modulo
  float MathematicalModulus(float a, float b) {
    return std::fmod((std::fmod(a, b) + b), b);
  }
}

Ppg::Ppg() {
  Reset();
}

void Ppg::Reset() {
  ready = false;
  hrsCount = 0;
  filteredHrsTailIndex = 0;
  filteredHrsArray.fill(0.f);
  accAdaptive.Reset();
  lockedHrBin = std::nullopt;
}

bool Ppg::SufficientData() const {
  return ready;
}

void Ppg::ScaleHrs(float scaleFactor) {
  ppgFilter.Scale(scaleFactor);
}

void Ppg::Ingest(uint16_t hrs, int16_t accX, int16_t accY, int16_t accZ) {
  // Acceleration is normalised to 1024=1G in the BMA driver
  float accXNorm = static_cast<float>(accX) / 1024.f;
  float accYNorm = static_cast<float>(accY) / 1024.f;
  float accZNorm = static_cast<float>(accZ) / 1024.f;
  // Filter initial states assume the previous input was zero
  // Instead prime the filters with a real input to avoid
  // ringing caused by a large jump from zero
  if (hrsCount == 0) {
    ppgFilter.Prime(hrs);
    accXFilter.Prime(accXNorm);
    accYFilter.Prime(accYNorm);
    accZFilter.Prime(accZNorm);
  }
  // Highpass the hrs to remove dc baseline
  float hrsFilt = ppgFilter.FilterStep(hrs);
  // Invert phase: normalise to positive pulse
  hrsFilt *= -1.f;

  filteredHrsArray[filteredHrsTailIndex] = std::abs(hrsFilt);
  filteredHrsTailIndex = (filteredHrsTailIndex + 1) % adaptiveResetWindow;

  // Highpass all the acceleration channels to remove dc baseline
  float accXFilt = accXFilter.FilterStep(accXNorm);
  float accYFilt = accYFilter.FilterStep(accYNorm);
  float accZFilt = accZFilter.FilterStep(accZNorm);
  std::optional<float> resetThresh = std::nullopt;
  if (hrsCount > adaptiveResetWindow) {
    resetThresh = 0.f;
    for (size_t i = 0; i < adaptiveResetWindow; i++) {
      resetThresh = std::max(resetThresh.value(), filteredHrsArray[i]);
    }
    resetThresh.value() *= adaptiveResetThresh;
  }
  std::array<float, noiseChannels> filteredAcc {accXFilt, accYFilt, accZFilt};
  // Motion adaptive filtering to remove components in the hrs signal caused by motion
  float hrsAdaptive = accAdaptive.FilterStep(hrsFilt, filteredAcc, resetThresh);
  StoreHrs(hrsAdaptive);
}

std::optional<uint8_t> Ppg::HeartRate() {
  if (hrsCount == inputLength) {
    ready = true;
    RunAlg();
    // Roll back by overlapWindow
    for (size_t index = 0; index < inputLength - overlapWindow; index++) {
      adaptiveHrsArray[index] = adaptiveHrsArray[index + overlapWindow];
    }
    hrsCount -= overlapWindow;
  }
  if (lockedHrBin.has_value() && rollingEnergy > rollingEnergyDisplayMinimum) {
    return std::round((lockedHrBin.value() + minFrequencyBin) * binWidth * 60.f);
  }
  return {};
}

void Ppg::StoreHrs(float filteredHrs) {
  if (hrsCount < inputLength) {
    adaptiveHrsArray[hrsCount] = filteredHrs;
    hrsCount++;
  } else {
    for (size_t index = 0; index < inputLength - 1; index++) {
      adaptiveHrsArray[index] = adaptiveHrsArray[index + 1];
    }
    adaptiveHrsArray[inputLength - 1] = filteredHrs;
  }
}

void Ppg::RunAlg() {
  complexFftArray.fill(0.f);
  float mean = 0.f;
  for (size_t index = 0; index < inputLength; index++) {
    mean += adaptiveHrsArray[index];
  }
  mean /= static_cast<float>(inputLength);
  for (size_t index = 0; index < inputLength; index++) {
    fftArray[index] = adaptiveHrsArray[index] - mean;
  }
  // Normalise energy of each segment so algorithm is input amplitude invariant
  SegmentRMSNorm();
  // Transform to frequency domain so we can do spectral processing
  Utility::FFT::RealFFT(complexFftArray, realTwiddle, complexTwiddle);
  // Find peaks in the spectral magnitudes so we can identify possible heart rate peaks
  ProminenceFilter();
  // Filter peaks by how likely they are to be heart rates using harmonic information
  HarmonicFilter();

  // Then identify and track peaks over time
  // Readers note: I suggest collapsing the lambdas immediately below and reading the tracking logic first
  // then come back to these after

  // Largest element in array
  auto FindPeakIndex = [](std::span<const float> array) {
    float largest = array[0];
    size_t largestIndex = 0;
    for (size_t index = 1; index < array.size(); index++) {
      if (array[index] > largest) {
        largest = array[index];
        largestIndex = index;
      }
    }
    return largestIndex;
  };
  // Total energy of a peak taking into account energy spread into neighbour bins
  auto CalculateLogPeakEnergy = [](size_t bin, std::span<const float> roi) {
    size_t lowerBound = std::max(0, static_cast<ssize_t>(bin) - 1);
    size_t upperBound = std::min(bin + 2, roiLength);
    float acc = 0.f;
    for (size_t index = lowerBound; index < upperBound; index++) {
      acc += roi[index];
    }
    return std::log(acc);
  };
  // Peak energy relative to total energy
  auto PeakProminence = [](float peakEnergy, std::span<const float> roi) {
    float acc = 0.f;
    for (auto item : roi) {
      acc += item;
    }
    return peakEnergy / acc;
  };
  // Exact peak location by interpolating with neighbour bins
  auto PeakSearch = [](size_t bin, std::span<const float> roi) {
    std::optional<float> upperEnergy = std::nullopt;
    std::optional<float> lowerEnergy = std::nullopt;
    float centralEnergy = roi[bin];
    if (bin < roi.size() - 1) {
      upperEnergy = roi[bin + 1];
    }
    if (bin > 0) {
      lowerEnergy = roi[bin - 1];
    }
    float peakLocation = bin;
    if (upperEnergy.has_value()) {
      peakLocation += (upperEnergy.value() / centralEnergy) / 2.f;
    }
    if (lowerEnergy.has_value()) {
      peakLocation -= (lowerEnergy.value() / centralEnergy) / 2.f;
    }
    return peakLocation;
  };
  // Exponential moving average
  auto EmaStep = [](float state, float next, float alpha) {
    return (state * (1.f - alpha)) + (next * alpha);
  };
  // Strongest peak near to the previous peak
  auto TrackHr = [FindPeakIndex, PeakSearch](float lockedHrBin, std::span<const float> roi) {
    ssize_t centre = std::round(lockedHrBin);
    float deviation = ((lockedHrBin + minFrequencyBin) * 2.f * 0.1f) + 1.f;

    // Need an odd number of bins so window is centred
    // Avoid subsampling bins
    auto RoundToOdd = [](float val) {
      return (2 * static_cast<size_t>(std::floor(val / 2))) + 1;
    };

    ssize_t width = std::max(RoundToOdd(deviation), 3U);
    ssize_t offset = (width - 1) / 2;
    size_t minBin = std::max(0, -offset + centre);
    size_t maxBin = std::min(static_cast<ssize_t>(roiLength), -offset + centre + width);
    std::span<const float> searchRoi = roi.subspan(minBin, maxBin - minBin);
    size_t peak = FindPeakIndex(searchRoi);
    return PeakSearch(peak, searchRoi) + minBin;
  };

  std::span<const float> roi = fftArray.subspan(minFrequencyBin, roiLength);
  size_t bestIndex = FindPeakIndex(roi);
  float bestPeakEnergy = CalculateLogPeakEnergy(bestIndex, roi);
  std::optional<size_t> strongBin = std::nullopt;

  if (bestPeakEnergy > strongPeakEnergyMinimum) {
    if (PeakProminence(std::exp(bestPeakEnergy), roi) > strongPeakProminenceMinimum) {
      strongBin = bestIndex;
    }
  }

  // If there's a strong peak and no heart rate locked, or if the current locked peak is weak
  // accept the new peak and begin tracking
  // Otherwise continue tracking the existing peak
  if ((!lockedHrBin.has_value() || rollingEnergy < rollingEnergyWeak) && strongBin.has_value()) {
    lockedHrBin = PeakSearch(strongBin.value(), roi);
    rollingEnergy = bestPeakEnergy;
  } else if (lockedHrBin.has_value()) {
    float newLockBin = TrackHr(lockedHrBin.value(), roi);
    float lockEnergy = CalculateLogPeakEnergy(std::round(newLockBin), roi);
    // Only update the estimated heart rate if the new peak location has substantial energy
    // This helps avoid tracking noise when the signal quality transiently degrades
    if (lockEnergy > hrUpdateEnergyMinimum) {
      lockedHrBin = EmaStep(lockedHrBin.value(), newLockBin, hrMovementAlpha);
    }
  }

  // Update rolling energy (if currently tracking a heart rate)
  // The rolling energy provides information on the energy of the peak over time
  // If the peak becomes consistently too weak we stop tracking it
  if (lockedHrBin.has_value()) {
    float lockEnergy = CalculateLogPeakEnergy(std::round(lockedHrBin.value()), roi);
    rollingEnergy = EmaStep(rollingEnergy, lockEnergy, rollingEnergyAlpha);

    if (rollingEnergy < trackingEnergyMinimum) {
      lockedHrBin = std::nullopt;
    }
  }
}

[[gnu::noinline]] void Ppg::ProminenceFilter() {
  std::array<float, fftLength / 2> prominence;
  for (size_t index = minFrequencyBin - 1; index < fftLength / 2; index++) {
    prominence[index] = std::log(std::max(std::abs(complexFftArray[index]), logFloor));
  }
  prominenceFilter.Prime(prominence[minFrequencyBin - 1] * primingFactor);

  // Zero phase filter: forwards + backwards
  // IIR filters delay the signal, and we want to avoid shifting peaks up or down in frequency
  // So we run the filter both forwards and backwards so the delay in each direction cancels out
  // The result is zero delay (phase shift) and twice the attenuation

  // forwards pass
  for (size_t index = minFrequencyBin; index < fftLength / 2; index++) {
    prominence[index] = prominenceFilter.FilterStep(prominence[index]);
  }
  // backwards pass
  for (size_t index = (fftLength / 2) - 1; index > minFrequencyBin - 1; index--) {
    prominence[index] = prominenceFilter.FilterStep(prominence[index]);
  }
  for (size_t index = minFrequencyBin; index < fftLength / 2; index++) {
    float val = (prominence[index] - minProminence) / (maxProminence - minProminence);
    val = std::max(std::min(1.f, val), logFloor);
    complexFftArray[index] *= val;
  }
}

[[gnu::noinline]] void Ppg::SegmentRMSNorm() {
  float rollSum = 0.0;
  for (size_t index = 0; index < segmentNormWindow; index++) {
    rollSum += std::pow(fftArray[index], 2);
  }
  std::array<float, inputLength - segmentNormWindow + 1> outputs;
  outputs[0] = std::sqrt(rollSum / segmentNormWindow) * 2;
  for (size_t index = segmentNormWindow; index < inputLength; index++) {
    rollSum -= std::pow(fftArray[index - segmentNormWindow], 2);
    rollSum += std::pow(fftArray[index], 2);
    outputs[index - segmentNormWindow + 1] = std::sqrt(rollSum / segmentNormWindow) * 2;
  }
  size_t halfNorm = segmentNormWindow / 2;
  for (size_t index = 0; index < inputLength; index++) {
    if (index <= halfNorm) {
      fftArray[index] /= outputs[0];
    } else if (index >= inputLength - halfNorm) {
      fftArray[index] /= outputs[outputs.size() - 1];
    } else {
      fftArray[index] /= outputs[index - halfNorm];
    }
  }
}

void Ppg::HarmonicFilter() {
  // These arrays are precomputed from clean PPG signals sampled from the sensor
  // These constants encode the shape of a heart beat on a PPG trace (beat morphology)
  // We use these constants to penalise components in the signal that don't correspond to a wave
  // of the right shape, allowing us to eliminate noise
  static constexpr std::array<std::array<float, 2>, Ppg::roiLength> magArray {
    {{0.3486803888990022, 0.255141508669598},    {0.36864722158239355, 0.25164382316780665}, {0.36771410549818545, 0.246948695573225},
     {0.35975580379324007, 0.24714886736121827}, {0.3658843568236732, 0.24650278235091835},  {0.3648928604195185, 0.24415607374450915},
     {0.3751222377393388, 0.23988295745228638},  {0.3921596966191289, 0.23210350117630843},  {0.40638499674260403, 0.22516855732998384},
     {0.4232127690875433, 0.21887312064931116},  {0.41564130033821856, 0.20673749156382631}, {0.38906619325409764, 0.1936140541005159},
     {0.3672394339754485, 0.18042745108982736},  {0.35275401585397503, 0.17151514618926156}, {0.3354326028852867, 0.15978169786664942},
     {0.32178572197981903, 0.1488045144540834},  {0.31281592316734735, 0.13991117977784315}, {0.299797871716369, 0.12788875993747936},
     {0.2966269594265519, 0.11875935055909558},  {0.2925247464787818, 0.11318066458803003},  {0.2964994671856146, 0.10580432119722558},
     {0.29770787961626416, 0.09839859522862732}, {0.3007591380586962, 0.09394035889667726},  {0.31104783374357775, 0.08933500974545709},
     {0.31486127213961, 0.08685088306152053},    {0.3313907046528911, 0.0924341145096451},   {0.30897754578100955, 0.08563613561250377},
     {0.3205154251103728, 0.08487605022457582},  {0.32285950835530364, 0.08122541502849505}, {0.31912839897477796, 0.08362307999351845},
     {0.30726200775051915, 0.08424021081449604}, {0.32024159984649864, 0.08405258601108509}, {0.33036938914308, 0.08392491126815374},
     {0.30531854817881166, 0.08146558813586019}, {0.3308979795151278, 0.08381011591996054},  {0.30632137960394173, 0.08313313923993279}}};

  static constexpr std::array<float, Ppg::roiLength> phaseArray {
    {0.2705229340875336, 0.36779124535955804, 0.45434779541743203, 0.5138242784968899, 0.5537065717483055,  0.5891055187012357,
     0.6221592074197645, 0.6558273339604292,  0.6849101620718369,  0.7083511816762154, 0.7167869997820329,  0.7112028603271641,
     0.7005327637607478, 0.6910573759719243,  0.6803636809415491,  0.6647598810363644, 0.6514913727959067,  0.6366402208133636,
     0.6210659369643988, 0.6057153725313422,  0.5894965013729228,  0.5757024639742967, 0.57176394709517514, 0.5881247512605522,
     0.6067800339551624, 0.621127310629737,   0.6243531002213607,  0.6276813318132224, 0.6314029448574778,  0.634138691540482,
     0.6372739733495546, 0.6398513675481557,  0.642109540006218,   0.6449694089531159, 0.6465457224589797,  0.649148754973601}};

  for (size_t frequencyBin = minFrequencyBin; frequencyBin < maxFrequencyBin; frequencyBin++) {
    float f0Mag = std::abs(complexFftArray[frequencyBin]);
    float f1Mag = (0.5f * std::abs(complexFftArray[(frequencyBin * 2) - 1])) + std::abs(complexFftArray[frequencyBin * 2]) +
                  (0.5f * std::abs(complexFftArray[(frequencyBin * 2) + 1]));
    float f2Mag = std::abs(complexFftArray[(frequencyBin * 3) - 1]) + std::abs(complexFftArray[frequencyBin * 3]) +
                  std::abs(complexFftArray[(frequencyBin * 3) + 1]);

    float f1MagRel = f1Mag / f0Mag;
    float f2MagRel = f2Mag / f0Mag;
    float f1MagPriorRel = f1MagRel / magArray[frequencyBin - minFrequencyBin][0];

    // Returns a phase between 0 and 1
    auto ComplexToPhaseNorm = [](std::complex<float> value) {
      return (std::arg(value) / (2.f * static_cast<float>(std::numbers::pi))) + 0.5f;
    };
    // Calculates the distance between two phases, 0 being in-phase and 1 being antiphase
    auto PhaseDistance = [](float p1, float p2) {
      return 1.f - (std::abs(0.5f - std::abs(p1 - p2)) * 2.f);
    };

    float f0Phase = ComplexToPhaseNorm(complexFftArray[frequencyBin]);
    float f1Phase = ComplexToPhaseNorm(complexFftArray[frequencyBin * 2]);
    float f1PhaseOffset = MathematicalModulus(f1Phase - (2 * f0Phase), 1.f);

    float filteredEnergy = f0Mag;
    if (f1MagPriorRel > f1MagMinimum) {
      float f1PhaseDist = PhaseDistance(f1PhaseOffset, phaseArray[frequencyBin - minFrequencyBin]);
      filteredEnergy *= std::exp(-f1ExpAttenuationFactor * f1PhaseDist);
      if (f1MagRel > f1MagHigh) {
        filteredEnergy = std::min(filteredEnergy, fnMagHighEnergyCeiling);
      }
    } else {
      filteredEnergy *= noF1Penalty;
      filteredEnergy = std::min(filteredEnergy, noF1EnergyCeiling);
    }
    if (f2MagRel > f2MagHigh) {
      filteredEnergy = std::min(filteredEnergy, fnMagHighEnergyCeiling);
    }
    fftArray[frequencyBin] = filteredEnergy;
  }
}
