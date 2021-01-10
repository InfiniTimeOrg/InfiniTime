#include <cmath>
#include "Ptagc.h"

using namespace Pinetime::Controllers;

Ptagc::Ptagc(float start, float decay, float threshold) : peak{start}, decay{decay}, boost{1.0f/decay}, threshold{threshold} {

}

float Ptagc::Step(float spl) {
  if(std::abs(spl) > peak)
    peak *= boost;
  else
    peak *= decay;

  if((spl > (peak * threshold)) || (spl < (peak * -threshold)))
    return 0.0f;

  spl = 100.0f * spl / (2.0f * peak);
  return spl;
}
