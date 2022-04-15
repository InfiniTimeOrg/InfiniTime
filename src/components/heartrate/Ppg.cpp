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
Ppg::Ppg()
  : hpf {0.87033078, -1.74066156, 0.87033078, -1.72377617, 0.75754694},
    agc {20, 0.971, 2},
    lpf {0.11595249, 0.23190498, 0.11595249, -0.72168143, 0.18549138} {
}

  int Ppg::Compare(int8_t* d1,int shift, size_t count) {
    int e = 0;
    for (size_t i = 0; i < count; i++) {
      auto d = d1[getRingIndex(i+shift)] - d1[getRingIndex(i)];
      e += d * d;
    }
    return e;
  }

  int Ppg::CompareShift(int8_t* d, int shift, size_t count) {
    return Compare(d ,shift, count - shift);
  }
  int Ppg::Trough(int8_t* d, size_t size, uint8_t mn, uint8_t mx) {
    auto z2 = CompareShift(d, mn - 2, size);
    auto z1 = CompareShift(d, mn - 1, size);
    for (int i = mn; i < mx + 1; i++) {
      auto z = CompareShift(d, i, size);
      if (z2 > z1 && z1 < z)
        return i;
      z2 = z1;
      z1 = z;
    }
    return -1;
  }

int8_t Ppg::Preprocess(float spl) {
  spl -= offset;
  spl = hpf.Step(spl);
  spl = agc.Step(spl);
  spl = lpf.Step(spl);

  auto spl_int = static_cast<int8_t>(spl);

  data[dataIndex] = spl_int;
  dataIndex = (dataIndex+1)%Ppg::DATA_SIZE;
  return spl_int;
}

float Ppg::HeartRate() {
  if(data[DATA_SIZE-1] == 0 || dataIndex% Ppg::UPDATE_HEARTRATE_AFTER != 0){
    return 0;
  }

  NRF_LOG_INFO("PREPROCESS, offset = %d", offset);
  auto hr = ProcessHeartRate();
  return hr;
}
float Ppg::ProcessHeartRate() {
  int t0 = Trough(data.data(), DATA_SIZE, 7, 48);
  if (t0 < 0)
    return 0;

  int t1 = t0 * 2;
  t1 = Trough(data.data(), DATA_SIZE, (t1 - 5), (t1 + 5));
  if (t1 < 0)
    return 0;

  int t2 = (t1 * 3) / 2;
  t2 = Trough(data.data(), DATA_SIZE,(t2 - 5),(t2 + 5));
  if (t2 < 0)
    return 0;

  int t3 = (t2 * 4) / 3;
  t3 = Trough(data.data(), DATA_SIZE,(t3 - 4), (t3 + 4));
  if (t3 < 0)
    return (60 * 24 * 3) / (t2);

  return (60 * 24 * 4) / (t3);
}

//Gets the Index in the Ring Buffer which corresponds to the index, if it was a 0-based Array
//DataIndex is where there was last data written to, so if the arrray was full once, it marks the end of the array
//The next Index is the start then.
int Ppg::getRingIndex(int8_t index){
  return (index+dataIndex)%Ppg::DATA_SIZE;
}
void Ppg::SetOffset(uint16_t offset) {
  this->offset = offset;
  this->Reset();
}

void Ppg::Reset() {
  dataIndex = 0;
  //invalidates the current array
  data[DATA_SIZE-1] = 0;
}
