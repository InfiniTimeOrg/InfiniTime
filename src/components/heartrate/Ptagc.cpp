/*
  SPDX-License-Identifier: LGPL-3.0-or-later
  Original work Copyright (C) 2020 Daniel Thompson
  C++ port Copyright (C) 2021 Jean-François Milants
*/

#include "components/heartrate/Ptagc.h"
#include <cmath>

using namespace Pinetime::Controllers;

/** Original implementation from wasp-os : https://github.com/daniel-thompson/wasp-os/blob/master/wasp/ppg.py */
Ptagc::Ptagc(float start, float decay, float threshold) : peak {start}, decay {decay}, boost {1.0f / decay}, threshold {threshold} {
}

float Ptagc::Step(float spl) {
  if (std::abs(spl) > peak) {
    peak *= boost;
  } else {
    peak *= decay;
  }

  if ((spl > (peak * threshold)) || (spl < (peak * -threshold))) {
    return 0.0f;
  }

  spl = 100.0f * spl / (2.0f * peak);
  return spl;
}
