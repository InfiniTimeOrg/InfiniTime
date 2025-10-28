#include "displayapp/Colors.h"

#include <limits>

Colors::Color Colors::linear_gradient(Colors::Color startingColor, Colors::Color endingColor, uint8_t progress) {
  constexpr decltype(progress) maxProgress = std::numeric_limits<decltype(progress)>::max();
  return Colors::Color(((maxProgress - progress) * startingColor.red() + progress * endingColor.red()) / maxProgress,
                       ((maxProgress - progress) * startingColor.green() + progress * endingColor.green()) / maxProgress,
                       ((maxProgress - progress) * startingColor.blue() + progress * endingColor.blue()) / maxProgress);
}
