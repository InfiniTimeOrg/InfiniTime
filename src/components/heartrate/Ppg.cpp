/*
  SPDX-License-Identifier: LGPL-3.0-or-later
  Original work Copyright (C) 2020 Daniel Thompson
  C++ port Copyright (C) 2021 Jean-François Milants
*/

#include "components/heartrate/Ppg.h"
#include <vector>
#include <nrf_log.h>
using namespace Pinetime::Controllers;

/** Original implementation from wasp-os : https://github.com/daniel-thompson/wasp-os/blob/master/wasp/ppg.py */
namespace {
  int Compare(int8_t* d1, int8_t* d2, size_t count) {
    int e = 0;
    for (size_t i = 0; i < count; i++) {
      auto d = d1[i] - d2[i];
      e += d * d;
    }
    return e;
  }

  int CompareShift(int8_t* d, int shift, size_t count) {
    return Compare(d + shift, d, count - shift);
  }

  int Trough(int8_t* d, size_t size, uint8_t mn, uint8_t mx) {
    auto z2 = CompareShift(d, mn - 2, size);
    auto z1 = CompareShift(d, mn - 1, size);
    for (int i = mn; i < mx + 1; i++) {
      auto z = CompareShift(d, i, size);
      if (z2 > z1 && z1 < z) {
        return i;
      }
      z2 = z1;
      z1 = z;
    }
    return -1;
  }
}

Ppg::Ppg()
  : hpf {0.87033078, -1.74066156, 0.87033078, -1.72377617, 0.75754694},
    agc {20, 0.971, 2},
    lpf {0.11595249, 0.23190498, 0.11595249, -0.72168143, 0.18549138} {
}

int8_t Ppg::Preprocess(float spl) {
  spl -= offset;
  spl = hpf.Step(spl);
  spl = agc.Step(spl);
  spl = lpf.Step(spl);

  auto spl_int = static_cast<int8_t>(spl);

  if (dataIndex < 200) {
    data[dataIndex++] = spl_int;
  }
  return spl_int;
}

int Ppg::HeartRate() {
  if (dataIndex < 200) {
    return 0;
  }

  NRF_LOG_INFO("PREPROCESS, offset = %d", offset);
  auto hr = ProcessHeartRate();
  dataIndex = 0;
  return hr;
}

int Ppg::ProcessHeartRate() {
  int t0 = Trough(data.data(), dataIndex, 7, 48);
  if (t0 < 0) {
    return 0;
  }

  int t1 = t0 * 2;
  t1 = Trough(data.data(), dataIndex, t1 - 5, t1 + 5);
  if (t1 < 0) {
    return 0;
  }

  int t2 = (t1 * 3) / 2;
  t2 = Trough(data.data(), dataIndex, t2 - 5, t2 + 5);
  if (t2 < 0) {
    return 0;
  }

  int t3 = (t2 * 4) / 3;
  t3 = Trough(data.data(), dataIndex, t3 - 4, t3 + 4);
  if (t3 < 0) {
    return (60 * 24 * 3) / t2;
  }

  return (60 * 24 * 4) / t3;
}

void Ppg::SetOffset(uint16_t offset) {
  this->offset = offset;
  dataIndex = 0;
}

void Ppg::Reset() {
  dataIndex = 0;
}
