/*
  SPDX-License-Identifier: LGPL-3.0-or-later
  Original work Copyright (C) 2020 Daniel Thompson
  C++ port Copyright (C) 2021 Jean-François Milants
*/

#include "components/heartrate/Biquad.h"

using namespace Pinetime::Controllers;

/** Original implementation from wasp-os : https://github.com/daniel-thompson/wasp-os/blob/master/wasp/ppg.py */
Biquad::Biquad(float b0, float b1, float b2, float a1, float a2) : b0 {b0}, b1 {b1}, b2 {b2}, a1 {a1}, a2 {a2} {
}

float Biquad::Step(float x) {
  auto v1 = this->v1;
  auto v2 = this->v2;

  auto v = x - (a1 * v1) - (a2 * v2);
  auto y = (b0 * v) + (b1 * v1) + (b2 * v2);

  this->v2 = v1;
  this->v1 = v;

  return y;
}
