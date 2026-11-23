#include "displayapp/screens/MossSimulator.h"

// Remember refreshes happen at approx. 30 hertz. Draw enough tiles to make each square loop take about this many refreshes.
//  If you want a target overall time for the entire growing, use S*600/W where S is seconds and W is screen width in pixels.
//  (The formula is technically not fully accurate but there's so many approximates here that it doesn't matter.)
// Updates will slow down as needed to prevent resource starvation (prevents too many drawn tiles per refresh, and adjusts
//  refresh rate to give time for other processes to run)
#define TARGET_LOOP_REFRESH_COUNT 5

// Outer radius of the bite mark to leave in the moss when tapping the screen during the eating phase (in pixels).
// If this is low, it'll be really tedious to eat the moss.
// Must be <= 49. Realistically can go a little over without issues, but don't tempt fate.
#define MOSS_MUNCH_RADIUS 35

// How often, in ticks, to buzz while touch is held.
// Note that if this is more like a maximum allowed frequency of buzzes, so if it's high and the user releases and re-touches
//  within this timeframe, a new buzz will not happen on the re-touch.
#define MUNCH_BUZZ_FREQUENCY pdMS_TO_TICKS(75)

// The watch touch detection is a little iffy near the very edge. In lieu of making the munch radius large, push out the touch input
//  during the eating phase by this many pixels from center (interpolating between 0 push-out at center and this much push-out at edge).
// Keep this below munch radius to make it less noticeable.
// Is pretty noticeable in Infinisim no matter what.
#define EATING_PUSHOUT 15

// Allow this many pixels to be left on the field during munching. Recommended to keep this low, else it feels
//  weird when the moss gets finished despite the user not finishing everything.
// Must be <= screen width * screen height.
#define MUNCH_LENIENCY 2

// When entering the growing and storydisp phases, touch inputs are rejected until the user lifts their finger from the screen.
// This is an additional protection, where touch inputs are rejected for a certain amount of time as well.
#define TOUCH_REJECT_TIMEOUT pdMS_TO_TICKS(350)

// This is how many refreshes happen before considering touch as having been released.
// (If it's 1, then it will consider touch as having been released on the second refresh after release.)
// Set to 0 to disable.
#define TOUCH_RELEASE_LENIENCY 3

// How long, in ticks, the user has to hold tap for at program start to enter the debug scene change state
#define DEBUG_ENTRY_HOLD_TIME pdMS_TO_TICKS(1000)

namespace {
  // Unused, so it must be commented out else the compiler throws a hissy fit.
  /// Cosine with integer return (range [-32767, 32767])
  // int16_t Cosine(int16_t angle) {
  //   return _lv_trigo_sin(angle + 90);
  // }

  /// Sine with integer return (range [-32767, 32767])
  int16_t Sine(int16_t angle) {
    return _lv_trigo_sin(angle);
  }

  /// Cosine with floating point return  (range [-1., 1.])
  float CosineF(int16_t angle) {
    return (float) _lv_trigo_sin(angle + 90) / 32767.f;
  }

  /// Sine with floating point return (range [-1., 1.])
  float SineF(int16_t angle) {
    return (float) _lv_trigo_sin(angle) / 32767.f;
  }

  /// Modulo two int16_t values and return a positive value
  int16_t ModPos(int16_t num, int16_t denom) {
    return ((num %= denom) < 0) ? num + denom : num;
  }

  // Only used in Perlin noise generation
  /// Returns the signed decimal component of a floating point value
  /// Integer part from negative infinity, so -1.7 -> 0.3
  float FloatDecPart(float a) {
    float ipart;
    return a < 0 ? std::modf(a, &ipart) + 1 : std::modf(a, &ipart);
  }

  // Only used in Perlin noise generation
  /// Simple easing function between a and b. Mix 0.0 means full a, mix 1.0 means full b.
  float Smoothstep(float a, float b, float mix) {
    // 3*mix^2 - 2*mix^3
    mix = (3 - 2 * mix) * mix * mix;
    return (a * (1 - mix)) + (b * mix);
  }

  // Only used in Perlin noise generation
  /// Take an integer coordinate and turn it into an angle.
  /// @return An angle in range [0,360)
  int16_t MakeAngleFromCoord(uint8_t x, uint8_t y) {
    static constexpr uint8_t perlinHashTable[256] = {
      205, 193, 63,  54,  65,  190, 30,  137, 129, 32,  247, 221, 91,  202, 93,  102, 143, 68,  15,  236, 225, 55,  114, 57,  185, 51,
      252, 110, 123, 11,  154, 133, 101, 120, 141, 12,  7,   67,  136, 216, 27,  209, 160, 104, 44,  22,  235, 4,   179, 9,   35,  237,
      241, 37,  246, 142, 156, 89,  113, 149, 81,  176, 0,   78,  175, 47,  6,   157, 164, 198, 167, 98,  169, 74,  25,  5,   40,  146,
      148, 116, 239, 108, 203, 48,  90,  226, 144, 242, 64,  159, 92,  88,  132, 130, 119, 95,  204, 232, 103, 61,  206, 174, 62,  199,
      56,  109, 145, 168, 210, 53,  58,  82,  131, 255, 122, 250, 39,  126, 106, 16,  99,  83,  170, 177, 2,   105, 50,  84,  183, 66,
      224, 230, 218, 125, 195, 135, 75,  212, 31,  20,  45,  43,  33,  223, 52,  69,  23,  243, 19,  97,  152, 124, 158, 72,  150, 166,
      151, 161, 34,  191, 49,  77,  217, 178, 107, 229, 187, 253, 96,  147, 3,   8,   42,  172, 10,  59,  233, 94,  215, 13,  222, 197,
      238, 121, 211, 173, 41,  80,  115, 194, 200, 79,  208, 1,   234, 244, 100, 182, 251, 70,  26,  180, 188, 86,  240, 227, 28,  228,
      60,  134, 196, 220, 128, 14,  163, 153, 192, 36,  254, 38,  249, 155, 21,  201, 213, 76,  117, 171, 111, 85,  140, 112, 181, 245,
      46,  138, 17,  184, 71,  127, 73,  231, 207, 189, 186, 118, 219, 139, 248, 214, 165, 18,  24,  29,  87,  162};
    // Use the x and y coords to make a fast and Good Enough random number, take bottom 5 bits, and map that to roughly 360 degrees
    return (int16_t) (perlinHashTable[perlinHashTable[x] + y] & 31) * (int16_t) 11;
  }

  /// Calculates if a point is inside of a bite mark.
  /// @param testX The x position of the point being tested
  /// @param testY The y position of the point being tested
  /// @param centerX The x position of the center of the bite mark
  /// @param centerY The y position of the center of the bite mark
  /// @return True if the point is inside the bite mark, else False.
  bool IsPointInBiteMark(int16_t testX, int16_t testY, int16_t centerX, int16_t centerY) {
    const int relX = testX - centerX;
    const int relY = testY - centerY;
    // Below math breaks at relative coords (0, 0)
    if (relX == 0 && relY == 0) {
      return true;
    }
    const uint16_t angle = _lv_atan2(relX, relY);
    const uint16_t distanceSquared = (relX * relX) + (relY * relY);
    // below equation is doing this: ((abs(sin(angle * 4.5)) + 6)/7) * MUNCH_RADIUS
    // the 4.5 is 1/2 of the number of spokes (so here it has 9 spokes), and the 6 and 7 are how much to push the wave out by
    //  (so here the squiggle is in the outer 1/7 of the radius)
    const uint32_t targetDistance =
      (int32_t) MOSS_MUNCH_RADIUS * (abs((int32_t) Sine(angle * 9 / 2)) + (32767 * 6)) / ((int32_t) 32767 * 7);
    return distanceSquared < targetDistance * targetDistance;
  }
}

using namespace Pinetime::Applications::Screens;

// Constructor for gradient data. Common among most noise types.
GradientData::GradientData(float gradientStart,
                           float gradientEnd,
                           lv_color_t fromColor,
                           lv_opa_t fromAlpha,
                           lv_color_t toColor,
                           lv_opa_t toAlpha)
  : fromColor(fromColor), fromAlpha(fromAlpha), toColor(toColor), toAlpha(toAlpha) {
  this->gradientStart = static_cast<uint16_t>(gradientStart * std::numeric_limits<uint16_t>::max());
  this->gradientEnd = static_cast<uint16_t>(gradientEnd * std::numeric_limits<uint16_t>::max());
}

// Constructor for gradient data with only a single color.
GradientData::GradientData(float gradientStart, float gradientEnd, lv_color_t color, lv_opa_t alpha)
  : GradientData(gradientStart, gradientEnd, color, alpha, color, alpha) {
}

// Set clipping. Is a chainable function to allow easy reuse of gradients in texture construction.
GradientData& GradientData::SetClip(bool low, bool high) {
  this->clipLow = low;
  this->clipHigh = high;
  return *this;
}

// Re-set endpoint locations. Is a chainable function to allow easy reuse of gradients in texture construction.
GradientData& GradientData::SetEndpoints(float start, float end) {
  this->gradientStart = static_cast<uint16_t>(start * std::numeric_limits<uint16_t>::max());
  this->gradientEnd = static_cast<uint16_t>(end * std::numeric_limits<uint16_t>::max());
  return *this;
}

// Re-set colors. Is a chainable function to allow easy reuse of gradients in texture construction.
GradientData& GradientData::SetColors(lv_color_t start, lv_color_t end) {
  this->fromColor = start;
  this->toColor = end;
  return *this;
}

// Re-set colors to a single value. Is a chainable function to allow easy reuse of gradients in texture construction.
GradientData& GradientData::SetColor(lv_color_t color) {
  return SetColors(color, color);
}

// Re-set alphas. Is a chainable function to allow easy reuse of gradients in texture construction.
GradientData& GradientData::SetAlphas(lv_opa_t start, lv_opa_t end) {
  this->fromAlpha = start;
  this->toAlpha = end;
  return *this;
}

// Re-set alphas to a single value. Is a chainable function to allow easy reuse of gradients in texture construction.
GradientData& GradientData::SetAlpha(lv_opa_t alpha) {
  return SetAlphas(alpha, alpha);
}

// Perform color interpolation on the gradient. The range of the uint16_t represents the full range of the gradient.
// 'From' is the left side, 'to' is the right side, and values between them have higher numbers going to the right side.
ColorWithOpacity GradientData::Interpolate(uint16_t chosenValue) const {
  // Off the left end. Inclusivity is to prevent issues where max == min.
  if (chosenValue < gradientStart) {
    if (clipLow) {
      return {.color = LV_COLOR_BLACK, .alpha = 0};
    }
    return {.color = fromColor, .alpha = fromAlpha};
  }
  // Off the right end, or if gradient start and gradient end are the same then also treat it as the right endpoint
  if (chosenValue > gradientEnd || gradientStart == gradientEnd) {
    if (clipHigh) {
      return {.color = LV_COLOR_BLACK, .alpha = 0};
    }
    return {.color = toColor, .alpha = toAlpha};
  }
  // Inside the gradient, mix the two and return
  const uint8_t placeInGradient = (uint32_t) 255 * (chosenValue - gradientStart) / (uint32_t) (gradientEnd - gradientStart);

  // Mix the colors if needed
  lv_color_t mixColor;
  lv_opa_t mixAlpha;
  if (fromColor.full == toColor.full) {
    mixColor = fromColor;
  } else {
    mixColor = lv_color_mix(fromColor, toColor, 255 - placeInGradient);
  }
  if (fromAlpha == toAlpha) {
    mixAlpha = fromAlpha;
  } else {
    mixAlpha = LV_MATH_UDIV255(((uint16_t) fromAlpha * (255 - placeInGradient)) + ((uint16_t) toAlpha * placeInGradient));
  }

  // Construct final color and return
  return {mixColor, mixAlpha};
}

// Constructor for data to be used in various noise type texture layers
template <typename T>
TextureLayerDataBaseMovable<T>::TextureLayerDataBaseMovable(const GradientData& gradientData, uint16_t scale)
  : gradientData(gradientData), scaleX(scale), scaleY(scale) {
  if (scale == 0) {
    this->scaleX = 1;
    this->scaleY = 1;
  }
  this->shiftX = std::rand();
  this->shiftY = std::rand();

  // Prevent the texture seam from being visible.
  // This does bias the shifts, but it's by so little that it hardly matters.
  if (this->shiftX < std::numeric_limits<int16_t>::min() + LV_HOR_RES) {
    this->shiftX += LV_HOR_RES;
  }
  if (this->shiftY < std::numeric_limits<int16_t>::min() + LV_VER_RES) {
    this->shiftY += LV_VER_RES;
  }
}

// Allows the user to set the shift of the texture layer
template <typename T>
T& TextureLayerDataBaseMovable<T>::SetShift(int16_t x, int16_t y) {
  this->shiftX = x;
  this->shiftY = y;
  // Prevent the texture seam from being visible.
  // To prevent this kicking in, just limit the range yourself (std::rand() & 0x3FFF is plenty)
  if (this->shiftX < std::numeric_limits<int16_t>::min() + LV_HOR_RES) {
    this->shiftX += LV_HOR_RES;
  }
  if (this->shiftY < std::numeric_limits<int16_t>::min() + LV_VER_RES) {
    this->shiftY += LV_VER_RES;
  }
  return static_cast<T&>(*this);
}

// Allows the user to set the per-axis scale of the texture layer
template <typename T>
T& TextureLayerDataBaseMovable<T>::SetScale(uint16_t x, uint16_t y) {
  this->scaleX = x;
  this->scaleY = y;
  return static_cast<T&>(*this);
}

// Allows the user to re-set the scale of the texture layer
template <typename T>
T& TextureLayerDataBaseMovable<T>::SetScale(uint16_t scale) {
  return SetScale(scale, scale);
}

// Calculate an entire layer of the TextureLayer and UPDATE the buffer with it. Agnostic of the internal layer type.
// This is preferred to CalculatePixel particularly for Perlin type noise layers, since it enforces a structure which allows
//  CalculatePerlinPixel to cache values and be quite a bit faster than it might be otherwise.
void TextureLayer::CalculateLayer(lv_color_t* buffer, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2) const {
  const lv_coord_t width = x2 - x1 + 1;
  const lv_coord_t height = y2 - y1 + 1;
  // Main loop to update the buffer
  for (lv_coord_t x = 0; x < width; x++) {
    for (lv_coord_t y = 0; y < height; y++) {
      const ColorWithOpacity layerColor = CalculatePixel(x1 + x, y1 + y);
      buffer[y * width + x] = lv_color_mix(layerColor.color, buffer[y * width + x], layerColor.alpha);
    }
  }
}

// Calculate a single pixel of the TextureLayer and return it. Agnostic of the internal layer type.
ColorWithOpacity TextureLayer::CalculatePixel(lv_coord_t x, lv_coord_t y) const {
  // Return if pixel is out of bounds
  if (x < minXBound || x > maxXBound || y < minYBound || y > maxYBound) {
    return {LV_COLOR_BLACK, LV_OPA_0};
  }
  // As bad as this looks, this was the nicest approach I could think of.
  switch (noiseType) {
    case LayerNoise::Blank:
      return CalculateBlankPixel();
    case LayerNoise::Simple:
      return CalculateSimplePixel();
    case LayerNoise::Perlin:
      return CalculatePerlinPixel(x, y);
    case LayerNoise::ShapeSquare:
      return CalculateSquarePixel(x, y);
    case LayerNoise::ShapeTriangle:
      return CalculateTrianglePixel(x, y);
    case LayerNoise::ShapeCircle:
      return CalculateCirclePixel(x, y);
  }
  return {LV_COLOR_BLACK, LV_OPA_0};
}

// Allows clipping a texture early if it doesn't need to cover the entire screen.
// Meant to allow for fancier art, but can be used for optimization as well.
TextureLayer& TextureLayer::SetBounds(int16_t minX, int16_t maxX, int16_t minY, int16_t maxY) {
  this->minXBound = minX;
  this->maxXBound = maxX;
  this->minYBound = minY;
  this->maxYBound = maxY;
  return *this;
}

// Calculate pixel color for a Blank noise type layer
ColorWithOpacity TextureLayer::CalculateBlankPixel() const {
  const TextureLayerDataBlank data = std::any_cast<TextureLayerDataBlank>(textureLayerData);
  return {.color = data.color, .alpha = data.alpha};
}

// Calculate pixel color for a Simple noise type layer
ColorWithOpacity TextureLayer::CalculateSimplePixel() const {
  const TextureLayerDataSimple data = std::any_cast<TextureLayerDataSimple>(textureLayerData);
  const uint16_t randomResult = std::rand();
  return data.gradientData.Interpolate(randomResult);
}

// Calculate pixel color for a Perlin noise type layer
// Positive X goes to the right, positive Y goes down
ColorWithOpacity TextureLayer::CalculatePerlinPixel(lv_coord_t x, lv_coord_t y) const {
  const TextureLayerDataPerlin data = std::any_cast<TextureLayerDataPerlin>(textureLayerData);
  // Why is Perlin noise so painful

  // Apply the shift to the coordinates
  x -= data.shiftX;
  y -= data.shiftY;

  // Floating coordinate for requested point INSIDE the tile
  const float inTileX = FloatDecPart((float) x / (float) data.scaleX);
  const float inTileY = FloatDecPart((float) y / (float) data.scaleY);
  // Integer coordinate of the requested tile (these are specifically the top left coord)
  // Need to subtract one if coords are negative to compensate for division going toward zero instead of negative infinity
  const int32_t tileX = x >= 0 ? x / data.scaleX : x / data.scaleX - 1;
  const int32_t tileY = y >= 0 ? y / data.scaleY : y / data.scaleY - 1;

  // Saved values. Populated with correct data on the off chance that the first call of this function does make tileX and tileY both be 0.
  static int32_t saveTileX = 0;
  static int32_t saveTileY = 0;
  static float cosineTLAngle = CosineF(MakeAngleFromCoord(0, 0));
  static float sineTLAngle = SineF(MakeAngleFromCoord(0, 0));
  static float cosineTRAngle = CosineF(MakeAngleFromCoord(1, 0));
  static float sineTRAngle = SineF(MakeAngleFromCoord(1, 0));
  static float cosineBLAngle = CosineF(MakeAngleFromCoord(0, 1));
  static float sineBLAngle = SineF(MakeAngleFromCoord(0, 1));
  static float cosineBRAngle = CosineF(MakeAngleFromCoord(1, 1));
  static float sineBRAngle = SineF(MakeAngleFromCoord(1, 1));

  // Only update angles if needed
  if (tileX != saveTileX || tileY != saveTileY) {
    saveTileX = tileX;
    saveTileY = tileY;
    // Reuse angle variable on all corners for simplicity
    int16_t angle = MakeAngleFromCoord(tileX, tileY);
    cosineTLAngle = CosineF(angle);
    sineTLAngle = SineF(angle);

    angle = MakeAngleFromCoord(tileX + 1, tileY);
    cosineTRAngle = CosineF(angle);
    sineTRAngle = SineF(angle);

    angle = MakeAngleFromCoord(tileX, tileY + 1);
    cosineBLAngle = CosineF(angle);
    sineBLAngle = SineF(angle);

    angle = MakeAngleFromCoord(tileX + 1, tileY + 1);
    cosineBRAngle = CosineF(angle);
    sineBRAngle = SineF(angle);
  }

  // Calculate the dot products from the point inside the tile to each corner
  // Need to recalculate the dot products because inTileX and inTileY have almost definitely changed, unlike tileX and tileY
  const float dotProdTL = (cosineTLAngle * inTileX) + (sineTLAngle * inTileY);
  const float dotProdTR = (cosineTRAngle * (inTileX - 1)) + (sineTRAngle * inTileY);
  const float dotProdBL = (cosineBLAngle * inTileX) + (sineBLAngle * (inTileY - 1));
  const float dotProdBR = (cosineBRAngle * (inTileX - 1)) + (sineBRAngle * (inTileY - 1));

  // Calculate output from the Perlin noise
  float floatResult = Smoothstep(Smoothstep(dotProdTL, dotProdTR, inTileX), Smoothstep(dotProdBL, dotProdBR, inTileX), inTileY);

  // Since by default 2D Perlin only outputs between -sqrt(1/2) and sqrt(1/2)), need to multiply by sqrt(2) to get range -1 to 1
  floatResult *= std::numbers::sqrt2_v<float>;

  // Put result in range [0,1]
  floatResult = (floatResult + 1) / 2;

  // If computer math was perfect then this would be exactly in range [-1,1], but unfortunately it's using floats so I don't trust it.
  // With safeties, convert floatResult into the full range of a uint16_t and interpolate the gradient with that value.
  if (floatResult > 1.f) {
    return data.gradientData.Interpolate(std::numeric_limits<uint16_t>::max());
  }
  if (floatResult < 0.f) {
    return data.gradientData.Interpolate(0);
  }
  return data.gradientData.Interpolate((uint16_t) (floatResult * std::numeric_limits<uint16_t>::max()));
}

// Calculate pixel color for a SquareNoise "noise" type layer
ColorWithOpacity TextureLayer::CalculateSquarePixel(lv_coord_t x, lv_coord_t y) const {
  const TextureLayerDataSquare data = std::any_cast<TextureLayerDataSquare>(textureLayerData);

  // Pixel coordinates inside each repeating tile. All quadrants have been moved to the equivalent position in the top right quadrant.
  const uint16_t inTileX = std::abs(ModPos(x - data.shiftX, data.scaleX) - (int16_t) (data.scaleX / 2));
  const uint16_t inTileY = std::abs(ModPos(y - data.shiftY, data.scaleY) - (int16_t) (data.scaleY / 2));

  // Convert the pixel positions to a range of [0, 0xFFFF]
  // Since the top right quadrant is half the size of the scale variables, need to multiply by 2 as well
  const uint16_t inSquareX = ((uint32_t) inTileX * (uint32_t) 0x1FFFD) / (uint32_t) data.scaleX;
  const uint16_t inSquareY = ((uint32_t) inTileY * (uint32_t) 0x1FFFD) / (uint32_t) data.scaleY;

  // Use the result as the gradient
  return data.gradientData.Interpolate(std::max(inSquareX, inSquareY));
}

// Calculate pixel color for a TriangleNoise "noise" type layer
// Returns values <=0.5 for points inside the triangle in the square, and the rest is filled in with values >0.5
ColorWithOpacity TextureLayer::CalculateTrianglePixel(lv_coord_t x, lv_coord_t y) const {
  const TextureLayerDataTriangle data = std::any_cast<TextureLayerDataTriangle>(textureLayerData);

  // Pixel coordinates inside each repeating tile. Left half of the triangle gets mapped over to the right.
  // inTileX is <=ceil(data.scaleX/2), inTileY is <scaleY
  const uint16_t inTileX = std::abs(ModPos((x - data.shiftX) + (data.scaleX / 2), data.scaleX) - (int16_t) (data.scaleX / 2));
  const uint16_t inTileY = ModPos(y - data.shiftY, data.scaleY);

  // Convert the pixel positions to a range of [0, 0x7FFF].
  // Need max to be half of 0xFFFF for the final calculation, else it will overflow and cause unwanted banding.
  // Since X is half the size of Y, also need to multiply it by 2
  const uint16_t inSquareX = ((uint32_t) inTileX * (uint32_t) 0xFFFF) / (uint32_t) data.scaleX;
  const uint16_t inSquareY = ((uint32_t) inTileY * (uint32_t) 0x7FFF) / (uint32_t) data.scaleY;

  // Use the result as the gradient
  return data.gradientData.Interpolate(0xFFFF - inSquareY - inSquareX);
}

// Calculate pixel color for a CircleNoise "noise" type layer
ColorWithOpacity TextureLayer::CalculateCirclePixel(lv_coord_t x, lv_coord_t y) const {
  const TextureLayerDataCircle data = std::any_cast<TextureLayerDataCircle>(textureLayerData);

  // Pixel coordinates inside each repeating tile. All quadrants have been moved to the equivalent position in the top right quadrant.
  const uint16_t inTileX = std::abs(ModPos(x - data.shiftX, data.scaleX) - (int16_t) (data.scaleX / 2));
  const uint16_t inTileY = std::abs(ModPos(y - data.shiftY, data.scaleY) - (int16_t) (data.scaleY / 2));

  // Convert the pixel positions to a range of [0, 0xFFFF/sqrt(2)]
  // Putting it in this range means that the final circle gradient will have a radius of sqrt(2), which means that the entire
  //  0xFFFF width/height tile will be _inside_ of the circle. It's more awkward to use, but it's a continuous gradient through the whole
  //  tile. It's not an ideal solution and means that a circle inset in the tile needs a gradient end of 1/sqrt(2), but it's a solution.
  // scaleMultiplier is (2 * 0xFFFF) / sqrt(2) because inTileX and inTileY is half the size of a full tile, so it needs to be doubled.
  constexpr uint32_t scaleMultiplier = (double) 0x1FFFD / std::numbers::sqrt2_v<double>;
  const uint32_t inSquareX = ((uint32_t) inTileX * scaleMultiplier) / (uint32_t) data.scaleX;
  const uint32_t inSquareY = ((uint32_t) inTileY * scaleMultiplier) / (uint32_t) data.scaleY;

  // _lv_sqrt doesn't seem to work on 32 bit values, need to use floating point operations instead
  float result = std::sqrt((float) (inSquareX * inSquareX + inSquareY * inSquareY));
  result = std::min(result, (float) 0xFFFF);

  // Use the result as the gradient
  return data.gradientData.Interpolate((uint16_t) result);
}

// Get given a buffer and populate it with the texture described by the TextureGenerator.
// The passed coordinates are what part of the image is being requested. The buffer still gets populated starting from 0.
// Coordinates are inclusive.
void TextureGenerator::GetBlock(lv_color_t* buffer, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2) {
  const lv_coord_t width = x2 - x1 + 1;
  const lv_coord_t height = y2 - y1 + 1;
  std::fill_n(buffer, width * height, LV_COLOR_MAGENTA); // Magenta to make errors very visible
  for (TextureLayer& layer : layers) {
    layer.CalculateLayer(buffer, x1, y1, x2, y2);
  }
}

// Calculate a single pixel of the texture.
lv_color_t TextureGenerator::GetPixel(lv_coord_t x, lv_coord_t y) const {
  lv_color_t workingValue = LV_COLOR_MAGENTA; // Magenta to make errors very visible
  for (const TextureLayer& layer : layers) {
    const ColorWithOpacity layerResult = layer.CalculatePixel(x, y);
    workingValue = lv_color_mix(layerResult.color, workingValue, layerResult.alpha);
  }
  return workingValue;
}

// Construct a moss munch tracker object with the given size. Remember internally it's using a bit per coordinate, not a full byte.
MossMunchTracker::MossMunchTracker(const uint16_t width, const uint16_t height) : width(width), height(height) {
  gridByteSize = width * height / 8;
  // Add one to the size if somehow the screen size is not a multiple of 8
  if ((width * height) % 8 != 0) {
    gridByteSize++;
  }
  grid = new uint8_t[gridByteSize];
  Reset();
}

// Copy constructor to satisfy rule of 3.
MossMunchTracker::MossMunchTracker(MossMunchTracker& other) : width(other.width), height(other.height), mossMunched(other.mossMunched) {
  gridByteSize = other.gridByteSize;
  grid = new uint8_t[gridByteSize];
  for (uint16_t i = 0; i < gridByteSize; i++) {
    this->grid[i] = other.grid[i];
  }
}

// Destructor since tracker uses memory on heap.
MossMunchTracker::~MossMunchTracker() {
  delete[] grid;
}

// Assignment operator to satisfy rule of 3.
MossMunchTracker& MossMunchTracker::operator=(MossMunchTracker const& other) {
  if (this == &other) {
    return *this;
  }

  delete[] grid;
  width = other.width;
  height = other.height;
  mossMunched = other.mossMunched;
  gridByteSize = other.gridByteSize;
  grid = new uint8_t[gridByteSize];
  for (uint16_t i = 0; i < gridByteSize; i++) {
    this->grid[i] = other.grid[i];
  }
  return *this;
}

// Check if the tracked moss patch has been finished off.
bool MossMunchTracker::IsFinished() const {
  // Checks if enough pixels have been eaten.
  return mossMunched >= (uint32_t) ((width * height) - MUNCH_LENIENCY);
}

// Reset the tracker grid to be full of moss again
void MossMunchTracker::Reset() {
  std::fill_n(grid, gridByteSize, 0xFF);
  mossMunched = 0;
}

// Put a bite mark in the munch tracker at the given coordinates. Follows the same logic that drawing does, so there should
// be no discrepancies between what is displayed and what is tracked as eaten.
bool MossMunchTracker::Munch(lv_coord_t centerX, lv_coord_t centerY) {
  bool ateAnyMoss = false;
  for (int16_t y = std::max(centerY - MOSS_MUNCH_RADIUS, 0); y <= std::min(centerY + MOSS_MUNCH_RADIUS, width - 1); y++) {
    for (int16_t x = std::max(centerX - MOSS_MUNCH_RADIUS, 0); x <= std::min(centerX + MOSS_MUNCH_RADIUS, height - 1); x++) {
      if (IsPointInBiteMark(x, y, centerX, centerY)) {
        ateAnyMoss = MunchBit(x, y) || ateAnyMoss;
      }
    }
  }
  return ateAnyMoss;
}

// Munch a single bit of moss.
inline bool MossMunchTracker::MunchBit(lv_coord_t x, lv_coord_t y) {
  // Bounds checking is handled in Munch()
  const uint16_t targetItem = ((y * width) + x) >> 3; // divide by 8
  const uint8_t mask = 0x80 >> (x & 0b111);           // equivalent to 0x80 >> (x % 8)

  // If grid item is unset, don't need to set it again. Just return false.
  if ((grid[targetItem] & mask) == 0) {
    return false;
  }
  grid[targetItem] &= ~mask;
  mossMunched++;
  return true;
}

// Create a MossStory object. Manages the simple story that goes with the moss eating.
MossStory::MossStory() {
  // Initialize recentBeats and recentScenes correctly
  std::fill_n(recentBeats, 5, StoryBeat::Error);
  std::fill_n(recentScenes, 4, StoryScene::Error);
  recentScenes[3] = StoryScene::Forest;
  mossEaten = 0;
  // Unused text, gets replaced when story gets updated
  curStoryText = std::string();
  currentScene = StoryScene::Forest;
  curMossType = MossType::Error;
  timeInCurrentScene = 0;
  // And finally, update internal state
  const StoryBeat nextBeat = GetNextStoryBeat();
  UpdateCurrentVars(nextBeat);
}

// Updates relevant internal states
void MossStory::IncrementStory() {
  mossEaten++;
  timeInCurrentScene++;
  const StoryBeat nextBeat = GetNextStoryBeat();
  UpdateCurrentVars(nextBeat);
}

// Return the text corresponding to the current scene
const char* MossStory::GetCurrentSceneText() const {
  switch (currentScene) {
    case StoryScene::Error:
      return "#ff00ff ERROR";
    case StoryScene::Forest:
      return "#00a000 Forest#";
    case StoryScene::Cave:
      return "#808080 Cave#";
    case StoryScene::Civilization:
      return "#ff8000 Civilization#";
    case StoryScene::House:
      return "#00a0a0 House#";
    case StoryScene::DbgAllMoss:
      return "#ff8fcc DbgAllMoss";
    case StoryScene::Egg:
      return "#ffe395 Egg";
    default:
      return "#ff00ff UNKNOWN";
  }
}

// Update scene, cycling around at the end
void MossStory::DebugIncrementScene() {
  currentScene = (StoryScene) ((uint8_t) currentScene + 1);
  if (currentScene > StoryScene::Max) {
    currentScene = StoryScene::Min;
  }
  // Stupid hack to make DbgAllMoss work as expected
  // Setting to Max instead of Min because the moss type gets incremented during the UpdateCurrentVars() call if the current
  //  scene is DbgAllMoss so this way it overflows correctly to Min.
  if (currentScene == StoryScene::DbgAllMoss) {
    curMossType = MossType::Max;
  }
  // Update internal state
  const StoryBeat nextBeat = GetNextStoryBeat();
  UpdateCurrentVars(nextBeat);
}

// Check if a StoryBeat has happened recently
bool MossStory::IsInRecentBeats(StoryBeat storyBeat, uint16_t howRecent) const {
  if (howRecent > 5) {
    howRecent = 5;
  }
  for (int i = 5 - howRecent; i < 5; i++) {
    if (recentBeats[i] == storyBeat) {
      return true;
    }
  }
  return false;
}

// Count how many times a scene has happened recently
uint16_t MossStory::CountInRecentScenes(StoryScene storyScene, uint16_t howRecent) const {
  if (howRecent > 4) {
    howRecent = 4;
  }
  uint16_t count = 0;
  for (int i = 4 - howRecent; i < 4; i++) {
    if (recentScenes[i] == storyScene) {
      count++;
    }
  }
  return count;
}

// Helper function to allow condensing the GetNextStoryBeat*() functions SIGNIFICANTLY
void MossStory::PushIfNotHappenedRecently(std::vector<StoryBeat>& possibleNextBeats, StoryBeat storyBeat, uint16_t howRecent) const {
  if (!IsInRecentBeats(storyBeat, howRecent)) {
    possibleNextBeats.push_back(storyBeat);
  }
}

// Just test a percent chance
inline bool MossStory::PercentChance(const uint8_t chanceTrue) {
  return (std::rand() % 100) + 1 <= chanceTrue;
}

// Evaluates the current state and returns the next story beat. Does NOT update current state.
// Structured like this so if certain criteria are met, the correct StoryBeat can be returned immediately rather than
//  having to go through the random pool to get it returned.
MossStory::StoryBeat MossStory::GetNextStoryBeat() {
  switch (currentScene) {
    case StoryScene::Forest:
      return GetNextStoryBeatForest();
    case StoryScene::Civilization:
      return GetNextStoryBeatCivilization();
    case StoryScene::Cave:
      return GetNextStoryBeatCave();
    case StoryScene::House:
      return GetNextStoryBeatHouse();
    case StoryScene::DbgAllMoss:
      return GetNextStoryBeatAllMoss();
    case StoryScene::Egg:
      return GetNextStoryBeatEgg();
    default:
      break;
  }
  return StoryBeat::Error;
}

// Get a Forest type storybeat
MossStory::StoryBeat MossStory::GetNextStoryBeatForest() const {
  std::vector<StoryBeat> possibleNextBeats {};

  // Add mosses available anywhere
  StoryBeat addedContentReturn = PushSharedBeats(possibleNextBeats);
  if (addedContentReturn != StoryBeat::Error) {
    return addedContentReturn;
  }

  // Add mosses available in both Forest and Civilization
  addedContentReturn = PushSharedCivForestBeats(possibleNextBeats);
  if (addedContentReturn != StoryBeat::Error) {
    return addedContentReturn;
  }

  // Starting on the third moss, increase chance for leaving every 3 mosses, forcing it at 12
  for (int i = timeInCurrentScene - 3; i >= 0; i -= 3) {
    // Always have two exits in the pool, but swap one for the other if it's happened too often recently
    if (CountInRecentScenes(StoryScene::Cave, 4) >= 2) {
      possibleNextBeats.push_back(StoryBeat::ForestExitCiv);
      possibleNextBeats.push_back(StoryBeat::ForestExitCiv);
    } else if (CountInRecentScenes(StoryScene::Civilization, 4) >= 2) {
      possibleNextBeats.push_back(StoryBeat::ForestExitCave);
      possibleNextBeats.push_back(StoryBeat::ForestExitCave);
    } else {
      possibleNextBeats.push_back(StoryBeat::ForestExitCiv);
      possibleNextBeats.push_back(StoryBeat::ForestExitCave);
    }
  }
  if (timeInCurrentScene >= 12) {
    // If one of the scenes happened too many times recently, force going to the other
    if (CountInRecentScenes(StoryScene::Cave, 4) >= 2) {
      return StoryBeat::ForestExitCiv;
    }
    if (CountInRecentScenes(StoryScene::Civilization, 4) >= 2) {
      return StoryBeat::ForestExitCave;
    }
    // No forcing needed, just flip a coin
    if (PercentChance(50)) {
      return StoryBeat::ForestExitCave;
    }
    return StoryBeat::ForestExitCiv;
  }

  // Intro (will get re-shown after an overflow but nobody's gonna eat that much moss...)
  if (mossEaten == 0) {
    return StoryBeat::ForestIntro;
  }
  if (mossEaten == 1) {
    return StoryBeat::ForestIntroPT2;
  }
  // All other forest mosses
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::ForestWild);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::ForestBig);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::ForestMossMoss);
  // Mosses that I don't want people getting TOO early on
  if (mossEaten >= 5) {
    PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::Crypt);
    PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::ForestStars);
  }

  // Failsafe: if the list of next beats is empty, just add this one to it as backup
  if (possibleNextBeats.empty()) {
    possibleNextBeats.push_back(StoryBeat::ForestWild);
  }

  // Choose a random beat and return it
  return possibleNextBeats[std::rand() % possibleNextBeats.size()];
}

// Get a Civilization type storybeat
MossStory::StoryBeat MossStory::GetNextStoryBeatCivilization() const {
  std::vector<StoryBeat> possibleNextBeats {};

  // Add mosses available anywhere
  StoryBeat addedContentReturn = PushSharedBeats(possibleNextBeats);
  if (addedContentReturn != StoryBeat::Error) {
    return addedContentReturn;
  }

  // Add mosses available in both Forest and Civilization
  addedContentReturn = PushSharedCivForestBeats(possibleNextBeats);
  if (addedContentReturn != StoryBeat::Error) {
    return addedContentReturn;
  }

  // Starting on the third moss, increase chance for leaving every 3 mosses, forcing it at 12
  for (int i = timeInCurrentScene - 3; i >= 0; i -= 3) {
    // Always have two exits in the pool, but swap one for the other if it's happened too often recently
    if (CountInRecentScenes(StoryScene::House, 4) >= 2) {
      possibleNextBeats.push_back(StoryBeat::CivExitForest);
      possibleNextBeats.push_back(StoryBeat::CivExitForest);
    } else if (CountInRecentScenes(StoryScene::Forest, 4) >= 2) {
      possibleNextBeats.push_back(StoryBeat::CivExitHouse);
      possibleNextBeats.push_back(StoryBeat::CivExitHouse);
    } else {
      possibleNextBeats.push_back(StoryBeat::CivExitHouse);
      possibleNextBeats.push_back(StoryBeat::CivExitForest);
    }
  }
  if (timeInCurrentScene >= 12) {
    // If one of the scenes happened too many times recently, force going to the other
    if (CountInRecentScenes(StoryScene::House, 4) >= 2) {
      return StoryBeat::CivExitForest;
    }
    if (CountInRecentScenes(StoryScene::Forest, 4) >= 2) {
      return StoryBeat::CivExitHouse;
    }
    // No forcing needed, just flip a coin
    if (PercentChance(50)) {
      possibleNextBeats.push_back(StoryBeat::CivExitHouse);
    }
    possibleNextBeats.push_back(StoryBeat::CivExitForest);
  }

  // All other civilization mosses
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CivPool);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CivBread);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CivCultivated);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CivMousse);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CivIrradiated);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CivMossta);

  // Failsafe: if the list of next beats is empty, just add this one to it as backup
  if (possibleNextBeats.empty()) {
    possibleNextBeats.push_back(StoryBeat::Grass);
  }

  // Choose a random beat and return it
  return possibleNextBeats[std::rand() % possibleNextBeats.size()];
}

// Get a Cave type storybeat
MossStory::StoryBeat MossStory::GetNextStoryBeatCave() const {
  std::vector<StoryBeat> possibleNextBeats {};

  // Add mosses available anywhere
  StoryBeat addedContentReturn = PushSharedBeats(possibleNextBeats);
  if (addedContentReturn != StoryBeat::Error) {
    return addedContentReturn;
  }

  // Enable ability to leave on the third moss, increase chance at 5 (to 36% chance of leaving), and force leaving at 7
  // Don't need to check recent scenes, since this is the only way to get out of the cave
  if ((timeInCurrentScene >= 3 && PercentChance(20)) || (timeInCurrentScene >= 5 && PercentChance(20)) || timeInCurrentScene >= 7) {
    return StoryBeat::CaveExitForest;
  }
  // All other cave mosses
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CaveLichen);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CaveDarkMoss);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CaveDimLitMoss);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CaveGlowingMoss);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CaveZombie);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CaveMonster);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::CaveBubbly);

  // Failsafe: if the list of next beats is empty, just add this one to it as backup
  if (possibleNextBeats.empty()) {
    possibleNextBeats.push_back(StoryBeat::CaveLichen);
  }

  // Choose a random beat and return it
  return possibleNextBeats[std::rand() % possibleNextBeats.size()];
}

// Get a House type storybeat
MossStory::StoryBeat MossStory::GetNextStoryBeatHouse() const {
  std::vector<StoryBeat> possibleNextBeats {};

  // Add mosses available anywhere
  StoryBeat addedContentReturn = PushSharedBeats(possibleNextBeats);
  if (addedContentReturn != StoryBeat::Error) {
    return addedContentReturn;
  }

  // Enable ability to leave on the third moss, increase chance at 5 (to 36% chance of leaving), and force leaving at 7
  // Don't need to check recent scenes, since this is the only way to get out of the house
  if ((timeInCurrentScene >= 3 && PercentChance(20)) || (timeInCurrentScene >= 5 && PercentChance(20)) || timeInCurrentScene >= 7) {
    return StoryBeat::HouseExitCiv;
  }
  // All other house mosses
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::HouseShowerCurtain);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::HouseDigitalMoss);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::HouseTVStatic);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::HouseCarpet);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::HouseMystery);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::HouseAurora);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::HouseZombie);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::HouseBlanket);

  // Failsafe: if the list of next beats is empty, just add this one to it as backup
  if (possibleNextBeats.empty()) {
    possibleNextBeats.push_back(StoryBeat::HouseBlanket);
  }

  // Choose a random beat and return it
  return possibleNextBeats[std::rand() % possibleNextBeats.size()];
}

// Only returns StoryBeat::CycleMoss to simply cycle through moss
MossStory::StoryBeat MossStory::GetNextStoryBeatAllMoss() {
  return StoryBeat::CycleMoss;
}

// Only returns StoryBeat::Egg
MossStory::StoryBeat MossStory::GetNextStoryBeatEgg() {
  return StoryBeat::Egg;
}

// Add story beats shared between Forest and Civilization scenes
MossStory::StoryBeat MossStory::PushSharedCivForestBeats(std::vector<StoryBeat>& possibleNextBeats) const {
  // Nighttime event, with a 50% chance to insert a NighttimeSky beat between Nighttime and Sunrise
  if (IsInRecentBeats(StoryBeat::Nighttime, 1)) {
    if (PercentChance(50)) {
      return StoryBeat::NighttimeSky;
    }
    return StoryBeat::Sunrise;
  }
  if (IsInRecentBeats(StoryBeat::NighttimeSky, 1)) {
    return StoryBeat::Sunrise;
  }
  if (!IsInRecentBeats(StoryBeat::Sunrise)) {
    possibleNextBeats.push_back(StoryBeat::Nighttime);
  }
  // Suspicious Moss event
  if (IsInRecentBeats(StoryBeat::SusMoss, 1) && PercentChance(75)) {
    return StoryBeat::SusMossResult;
  }
  if (!IsInRecentBeats(StoryBeat::SusMoss)) {
    possibleNextBeats.push_back(StoryBeat::SusMoss);
  }
  // All other mosses
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::TreeMoss);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::Fiery);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::Grass);
  PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::SimpleMoss);
  // Mosses that I don't want people getting TOO early on
  if (mossEaten >= 5) {
    PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::MossHell);
    PushIfNotHappenedRecently(possibleNextBeats, StoryBeat::Supportive);
  }

  // Did not have a moss type that HAD to happen, so indicate as much
  return StoryBeat::Error;
}

// Add story beats shared between all scenes (mostly easter eggs)
MossStory::StoryBeat MossStory::PushSharedBeats(std::vector<StoryBeat>& possibleNextBeats) const {
  // Zero chance for these easter egg mosses if early in the story
  if (mossEaten < 10) {
    return StoryBeat::Error;
  }

  // Various easter eggs
  if (!IsInRecentBeats(StoryBeat::SpanishMossquisition) && PercentChance(10)) {
    possibleNextBeats.push_back(StoryBeat::SpanishMossquisition);
  }
  if (!IsInRecentBeats(StoryBeat::GoodLooking) && currentScene != StoryScene::Cave && PercentChance(7)) {
    possibleNextBeats.push_back(StoryBeat::GoodLooking);
  }
  if (!IsInRecentBeats(StoryBeat::LostTheGame) && PercentChance(7)) {
    possibleNextBeats.push_back(StoryBeat::LostTheGame);
  }
  // Flat 0.1% chance for golden moss
  if (!IsInRecentBeats(StoryBeat::GoldenMoss) && PercentChance(1) && PercentChance(10)) {
    return StoryBeat::GoldenMoss;
  }

  // Did not have a moss type that HAD to happen, so indicate as much
  return StoryBeat::Error;
}

// Updates internal variables with the correct text and moss type, and push the beat onto recentEvents
// Also manages changing the current scene
void MossStory::UpdateCurrentVars(StoryBeat storyBeat) {
  // Update recentEvents
  for (int i = 0; i < 4; i++) {
    recentBeats[i] = recentBeats[i + 1];
  }
  recentBeats[4] = storyBeat;

  // Update scene if the story beat warrants it
  bool sceneUpdated = false;
  if (storyBeat == StoryBeat::CivExitForest || storyBeat == StoryBeat::CaveExitForest) {
    currentScene = StoryScene::Forest;
    timeInCurrentScene = 0;
    sceneUpdated = true;
  } else if (storyBeat == StoryBeat::ForestExitCiv || storyBeat == StoryBeat::HouseExitCiv) {
    currentScene = StoryScene::Civilization;
    timeInCurrentScene = 0;
    sceneUpdated = true;
  } else if (storyBeat == StoryBeat::ForestExitCave) {
    currentScene = StoryScene::Cave;
    timeInCurrentScene = 0;
    sceneUpdated = true;
  } else if (storyBeat == StoryBeat::CivExitHouse) {
    currentScene = StoryScene::House;
    timeInCurrentScene = 0;
    sceneUpdated = true;
  }
  // Push currentScene onto recentScenes if scene was updated
  if (sceneUpdated) {
    for (int i = 0; i < 3; i++) {
      recentScenes[i] = recentScenes[i + 1];
    }
    recentScenes[3] = currentScene;
  }

  // Welcome, traveller, to THE BLOCK.
  // THE BLOCK means no harm. It is simply a result of me not really knowing how to make an automatic system dynamic enough
  //  to support all the features I need (mostly allowing sub-types of texts/mosses per storybeat) while retaining a semblance
  //  of usability and readability.
  // So instead, I have crafted THE BLOCK.
  // It is perfectly dynamic, since it is composed of ordinary code.
  // It is perfectly readable, since there's no strange code paths happening in here.
  // It is perfectly awful, because it's SO INCONCEIVABLY LONG.
  // Welcome, traveller, to THE BLOCK.

  // Each line of story should not be over 20 chars wide (guide: 123456789ABCDEFGHIJK) and 10 lines tall.
  // Recommended to stay under 8 (ideally 7) lines, since this text is centered vertically and 9 or more lines could
  //  overlap with the scene text.
  switch (storyBeat) {
    default:
    case StoryBeat::Error:
      // Should really very much absolutely never happen
      curStoryText = "ERROR";
      curMossType = MossType::Error;
      break;
    case StoryBeat::Nighttime:
      curStoryText = "Night falls, but\nyou keep munching.";
      curMossType = MossType::Dark;
      break;
    case StoryBeat::NighttimeSky:
      switch (std::rand() % 3) {
        default:
        case 0:
          curStoryText = "The stars tonight\nlook particularly\ndelectable.";
          curMossType = MossType::Stars;
          break;
        case 1:
          curStoryText = "You find a patch of\nmoss that's absorbed\ntoo much moonlight.";
          curMossType = MossType::Moon;
          break;
        case 2:
          curStoryText = "You look up to see\na delicious looking\naurora spread\nacross the sky.";
          curMossType = MossType::Aurora;
          break;
      }
      break;
    case StoryBeat::Sunrise:
      curStoryText = "The sun comes up\nover the horizon,\nilluminating a new\ntarget.";
      curMossType = MossType::Sunrise;
      break;
    case StoryBeat::SusMoss:
      curStoryText = "You come across\na patch of\nsuspicious\nlooking moss.";
      curMossType = MossType::Suspicious;
      break;
    case StoryBeat::SusMossResult:
      curStoryText = "Eating that last\nmoss wasn't a\ngood idea.";
      curMossType = MossType::Sickened;
      break;
    case StoryBeat::TreeMoss:
      curStoryText = "You find some\ndelicious moss\ngrowing on a\ntree.";
      curMossType = MossType::TreeMoss;
      break;
    case StoryBeat::Grass:
      curStoryText = "You couldn't find\nany moss, so you\nget friendly with\nsome grass\ninstead...";
      curMossType = MossType::Grass;
      break;
    case StoryBeat::Supportive:
      curStoryText = "You find a patch of\nparticularly\nsupportive moss!\nOr a melted flag,\nyou're really not\nsure.";
      switch (std::rand() % 9) {
        default:
        case 0:
        case 1:
          curMossType = MossType::QueerPride;
          break;
        case 2:
          curMossType = MossType::QueerTrans;
          break;
        case 3:
          curMossType = MossType::QueerLesbian;
          break;
        case 4:
          curMossType = MossType::QueerGay;
          break;
        case 5:
          curMossType = MossType::QueerBi;
          break;
        case 6:
          curMossType = MossType::QueerPan;
          break;
        case 7:
          curMossType = MossType::QueerEnby;
          break;
        case 8:
          curMossType = MossType::QueerAce;
          break;
      }
      break;
    case StoryBeat::Fiery:
      curStoryText = "You find a patch of\nflaming moss! You\nneed to extinguish\nit quickly! With\nyour mouth!";
      curMossType = MossType::Fiery;
      break;
    case StoryBeat::MossHell:
      curStoryText = "A portal to Moss\nHell opens next to\nyou. You poke your\nhead in to nibble\nthe nearest moss.";
      curMossType = MossType::RedGlow;
      break;
    case StoryBeat::SimpleMoss:
      curStoryText = "You find a normal\npatch of moss, then\ndevour it.";
      curMossType = MossType::Basic;
      break;
    case StoryBeat::HouseExitCiv: {
      const uint8_t choice = std::rand() % 5;
      // 2:2:1 chances for the following results
      if (choice < 2) {
        curStoryText = "You've had enough\nof the mosses in\nthis house. You\nleave through the\nwindow.";
        curMossType = MossType::Glass;
      } else if (choice < 4) {
        curStoryText = "You've had enough\nof the mosses in\nthis house. You\ndo what you must\nin order to leave.";
        curMossType = MossType::Brick;
      } else {
        curStoryText = "You think of all\nthe mosses you\nmight have missed\nin the house.\nSo you eat\nthe house.";
        curMossType = MossType::EntireHouse;
      }
      break;
    }
    case StoryBeat::HouseShowerCurtain:
      curStoryText = "You find a damp\ntapestry of moss\ndangling from a pole\nnext to someone's\nbathtub.";
      curMossType = MossType::ShowerCurtain;
      break;
    case StoryBeat::HouseDigitalMoss:
      curStoryText = "You find a box with\nsome IOT moss in\nit. You don't know\nwhat that means.";
      curMossType = MossType::Digital;
      break;
    case StoryBeat::HouseTVStatic:
      curStoryText = "A nearby TV hisses\nwith all the voices\nof moss. You eat it.";
      curMossType = MossType::TVStatic;
      break;
    case StoryBeat::HouseCarpet:
      curStoryText = "You find some moss\non the floor of the\nliving room.";
      curMossType = MossType::Carpet;
      break;
    case StoryBeat::HouseMystery:
      curStoryText = "A strangely\ndark room.";
      curMossType = MossType::Dark;
      break;
    case StoryBeat::HouseAurora:
      curStoryText = "You enter the\nkitchen and find...\naurora borealis?";
      curMossType = MossType::Aurora;
      break;
    case StoryBeat::HouseZombie:
      curStoryText = "A basement with\nzombies in it.\nThere is moss on\nthem.";
      curMossType = MossType::Zombie;
      break;
    case StoryBeat::HouseBlanket:
      curStoryText = "You come across a\nblanket of moss.\nOr is it a normal\nblanket? It's hard\nto tell.";
      curMossType = MossType::Blanket;
      break;
    case StoryBeat::ForestIntro:
      curStoryText = "You want to\neat moss.\n\n(Tap to continue)";
      curMossType = MossType::Basic;
      break;
    case StoryBeat::ForestIntroPT2:
      curStoryText = "Licking your lips,\nyou move on to the\nnext moss.";
      curMossType = MossType::Basic;
      break;
    case StoryBeat::ForestWild:
      curStoryText = "A patch of wild\nmoss catches your\neye.";
      curMossType = MossType::Wild;
      break;
    case StoryBeat::ForestBig:
      curStoryText = "You wander too far\ninto the forest,\nand find moss bigger\nthan you thought\npossible.";
      curMossType = MossType::Big;
      break;
    case StoryBeat::ForestMossMoss:
      curStoryText = "You find a patch\nof moss growing on\nmore moss.\nCool!";
      curMossType = MossType::MossMoss;
      break;
    case StoryBeat::Crypt:
      // mossEaten will always fit, since as a 16 bit value it can be at most 5 chars long.
      curStoryText = "You find an ancient\ncrypt holding the\nsouls of the ";
      curStoryText += std::to_string(mossEaten);
      curStoryText += "\nmosses you've\neaten.";
      curMossType = MossType::Soul;
      break;
    case StoryBeat::ForestExitCiv:
      curStoryText = "You find your\nway out of the\nforest and into\ncivilization, but\nfirst you take a\npiece of goodbye\nmoss.";
      switch (std::rand() % 3) {
        default:
        case 0:
          curMossType = MossType::Wild;
          break;
        case 1:
          curMossType = MossType::Basic;
          break;
        case 2:
          curMossType = MossType::TreeMoss;
          break;
      }
      break;
    case StoryBeat::ForestExitCave:
      curStoryText = "You find a cave.\nSurely there's some\ninteresting moss\ninside!";
      curMossType = MossType::Gray;
      break;
    case StoryBeat::ForestStars:
      curStoryText = "A door with no\nhouse behind it. It\nyields to a field\nof stars.";
      curMossType = MossType::Stars;
      break;
    case StoryBeat::CivPool:
      curStoryText = "You fall into\nsomeone's pool, but\ndiscover moss\ngrowing on the\nbottom!";
      curMossType = MossType::Blue;
      break;
    case StoryBeat::CivBread:
      curStoryText = "You find some\nstrange looking loaf\nof tan moss someone\nleft out. You think\nit's moss anyway.";
      curMossType = MossType::Bread;
      break;
    case StoryBeat::CivCultivated:
      curStoryText = "You find someone\nwho's been\ncultivating moss.\nYou free them of\ntheir burden.";
      curMossType = MossType::Cultivated;
      break;
    case StoryBeat::CivMousse:
      curStoryText = "You find some\ndelicious mousse.\nWait- that's what\nyou wanted, right?";
      curMossType = MossType::Mousse;
      break;
    case StoryBeat::CivExitForest:
      curStoryText = "The great forest\nnearby beckons you\nwith promises of\nmoss. You still take\nsome moss for the\nroad.";
      switch (std::rand() % 2) {
        default:
        case 0:
          curMossType = MossType::Wild;
          break;
        case 2:
          curMossType = MossType::TreeMoss;
          break;
      }
      break;
    case StoryBeat::CivExitHouse:
      switch (std::rand() % 2) {
        default:
        case 0:
          curStoryText = "You find a house\nthat looks ripe with\nmoss. You chew\nthrough the window.";
          curMossType = MossType::Glass;
          break;
        case 1:
          curStoryText = "You find a house\nthat looks ripe with\nmoss. You enter\nthrough the wall.";
          curMossType = MossType::Brick;
          break;
      }
      break;
    case StoryBeat::CivIrradiated:
      curStoryText = "You find some glowy\nmoss with some weird\nfidget spinner\nsignage next to it.";
      curMossType = MossType::Irradiated;
      break;
    case StoryBeat::CivMossta:
      curStoryText = "You find a lovely\nplate of mossta.";
      curMossType = MossType::Mossta;
      break;
    case StoryBeat::CaveLichen:
      curStoryText = "You fail to find\nmoss. Disheartened,\nyou nibble some\nlichen instead.";
      curMossType = MossType::Lichen;
      break;
    case StoryBeat::CaveDarkMoss:
      curStoryText = "You find some moss\nnestled deep in a\ncorner.";
      curMossType = MossType::Dark;
      break;
    case StoryBeat::CaveDimLitMoss:
      curStoryText = "You find some\nmoss growing in\nthe dim light.";
      curMossType = MossType::Gray;
      break;
    case StoryBeat::CaveGlowingMoss:
      curStoryText = "A patch of\nbeautiful glowing\nmoss catches your\neye.";
      switch (std::rand() % 2) {
        default:
        case 0:
          curMossType = MossType::DarkGlowyBlue;
          break;
        case 1:
          curMossType = MossType::DarkGlowyOrange;
          break;
      }
      break;
    case StoryBeat::CaveZombie:
      curStoryText = "A patch of dead...\nNo, UNdead moss\ncatches your eye!\nIt's zombie moss!";
      curMossType = MossType::Zombie;
      break;
    case StoryBeat::CaveMonster:
      curStoryText = "You meet a cool\ncave monster.\nYou dine together.";
      curMossType = MossType::Monster;
      break;
    case StoryBeat::CaveExitForest:
      curStoryText = "You finally find\nyour way out of the\ndamp cave. Some lush\nmoss greets you at\nthe exit.";
      curMossType = MossType::MossMoss;
      break;
    case StoryBeat::CaveBubbly:
      curStoryText = "You find a bubbling\npatch of moss deep\nin the caves.";
      curMossType = MossType::DarkBubbly;
      break;
    case StoryBeat::SpanishMossquisition:
      curStoryText = "Nobody expects\nthe Spanish\nMossquisition!";
      curMossType = MossType::SpanishMossquisition;
      break;
    case StoryBeat::GoodLooking:
      curStoryText = "A particularly good\nlooking piece of\nmoss catches your\neye.";
      curMossType = MossType::GoodLooking;
      break;
    case StoryBeat::GoldenMoss:
      curStoryText = "You find the\nincredibly rare\nGolden Moss!\nHoly badonkers!";
      curMossType = MossType::Gold;
      break;
    case StoryBeat::LostTheGame:
      curStoryText = "You just lost\nThe Game.";
      curMossType = MossType::Frowny;
      break;
    case StoryBeat::CycleMoss:
      curMossType = (MossType) ((uint8_t) curMossType + 1);
      if (curMossType > MossType::Max) {
        curMossType = MossType::Min;
      }
      // If finished off all moss, give a special text
      if (curMossType == MossType::AteAllMoss) {
        curStoryText = "There's no more\nmoss in sight. You\nBLJ into a parallel\nuniverse to get more\n"
                       "moss, but not before\neating your reward.";
      } else {
        switch (std::rand() % 6) {
          default:
          case 0:
            curStoryText = "MOSS PARTY!!!";
            break;
          case 1:
            curStoryText = "A lovely\narrangement of\nall your favorite\nmosses.";
            break;
          case 2:
            curStoryText = "Finishing that\nmoss, a new one\nmaterializes in\nfront of you.";
            break;
          case 3:
            curStoryText = "Too many mosses,\nnot enough time.";
            break;
          case 4:
            curStoryText = "Your hunger cannot\nbe sated, but the\nGods still try.";
            break;
          case 5:
            curStoryText = "The scent of too\nmany mosses assail\nyour senses.\nYou can only grin.";
            break;
        }
      }
      // Thank you, C++ strings, for allowing me to do stupid things like this.
      curStoryText = curStoryText + "\n(" + std::to_string((uint8_t) curMossType) + ")";
      break;
    case StoryBeat::Egg: {
      switch (std::rand() % 7) {
        default:
        case 0:
          curStoryText = "A strange little\nsecret?";
          break;
        case 1:
          curStoryText = "Smells like\npainted eggs.";
          break;
        case 2:
          curStoryText = "The sheer number\nof eggs makes your\nhead spin.\nYou continue your\nwork.";
          break;
        case 3:
          curStoryText = "A lovely egg\nfor a lovely person\n:)";
          break;
        case 4:
          curStoryText = "Colorful stripes\nand spots fill\nyour vision.";
          break;
        case 5:
          curStoryText = "You found\nan egg!";
          break;
        case 6:
          curStoryText = "The eggs\nfound you.";
          break;
        case 7: {
          // Creates a 19x7 char string of "Egg", joined with random chars from ".?+!!~"
          curStoryText = "";
          int outerIdx = 0;
          while (true) {
            for (int i = 0; i < 4; i++) {
              curStoryText = curStoryText + "Egg" + ".?+!!~"[std::rand() % 6];
            }
            curStoryText += "Egg";
            if (outerIdx >= 7) {
              break;
            }
            outerIdx++;
            curStoryText += "\n";
          }
          break;
        }
      }
      curMossType = MossType::Egg;
    }
  }
}

// The constructor for the main app object.
MossSimulator::MossSimulator(Components::LittleVgl& lvgl, Controllers::MotorController& motorController, System::SystemTask& systemTask)
  : munchTracker(LV_HOR_RES, LV_VER_RES), lvgl(lvgl), motorController(motorController), wakeLock(systemTask) {
  // Acquire wake lock for the duration of the application
  wakeLock.Lock();

  // Misc data setup
  storyText = lv_label_create(lv_scr_act(), nullptr);
  sceneText = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(sceneText, true);
  lastEatingBuzzTime = xTaskGetTickCount();
  std::srand(xTaskGetTickCount());
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  // Initialize buffers. Not needed, but let's be nice.
  std::fill_n(buffer1, 100, LV_COLOR_MAGENTA);
  std::fill_n(buffer2, 100, LV_COLOR_MAGENTA);

  // Start the story
  mossStory = MossStory();

  // Initial phase is DbgEntry, perform tasks needed for that
  // This scene either updates to DbgSceneSelect or StoryDisp depending on if user held screen.
  // DbgSceneSelect updates to StoryDisp once it's done.
  SwitchToDbgEntry();
}

MossSimulator::~MossSimulator() {
  // Release wake lock and delete data that needs to be explicitly deleted
  wakeLock.Release();
  lv_obj_clean(lv_scr_act());
  lv_task_del(taskRefresh);
}

// Handle tap inputs.
bool MossSimulator::OnTouchEvent(uint16_t x, uint16_t y) {
  switch (state) {
    case Growing:
      // If user is still holding tap from previous state, ignore it until they release the tap
      if (stillHoldingTap) {
        return true;
      }
      // Switch to StoryDisp state and yell at user
      lv_label_set_text_static(sceneText, mossStory.GetCurrentSceneText());
      lv_label_set_text_static(storyText, "Please respect\nthe moss while\nit grows.");
      SwitchToStoryDisp();
      break;
    case Eating: {
      // If user has held tap all the way through the growing phase, continue ignoring it here
      if (stillHoldingTap) {
        return true;
      }
      // Push out from center
      int16_t pushedX = (int16_t) x - LV_HOR_RES / 2;
      int16_t pushedY = (int16_t) y - LV_VER_RES / 2;
      pushedX = (pushedX * (EATING_PUSHOUT + (LV_HOR_RES / 2))) / (LV_HOR_RES / 2) + (LV_HOR_RES / 2);
      pushedY = (pushedY * (EATING_PUSHOUT + (LV_VER_RES / 2))) / (LV_VER_RES / 2) + (LV_VER_RES / 2);
      const bool ateAnyMoss = munchTracker.Munch(pushedX, pushedY);
      // Only draw the bite mark if moss has been eaten
      if (ateAnyMoss) {
        DrawBiteMark(pushedX, pushedY);
      }
      // If enough time has passed since last buzz (and user ate some moss), do another buzz
      if (ateAnyMoss && xTaskGetTickCount() - lastEatingBuzzTime >= MUNCH_BUZZ_FREQUENCY) {
        motorController.RunForDuration(10);
        lastEatingBuzzTime = xTaskGetTickCount();
      }
      break;
    }
    case StoryDisp:
      // Wait until user releases tap before allowing tapping to continue
      if (stillHoldingTap) {
        return true;
      }
      // User tapped after having released tap, so switch to Growing state
      SwitchToGrowing();
      break;
    case DbgSceneSelect:
      // If user taps screen in DbgSceneSelect, just continue past
      lv_label_set_text_static(storyText, mossStory.GetCurrentStoryText());
      lv_label_set_text_static(sceneText, mossStory.GetCurrentSceneText());
      SwitchToStoryDisp();
      break;
    default:
      break;
  }
  return true;
}

// Cycle scene if in the DbgSceneSelect state, else do nothing
bool MossSimulator::OnButtonPushed() {
  if (state == DbgSceneSelect) {
    // Cycle scene and update text
    mossStory.DebugIncrementScene();
    lv_label_set_text_static(sceneText, mossStory.GetCurrentSceneText());
    lv_label_set_align(sceneText, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(sceneText, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
    return true;
  }
  return false;
}

// Simple function which delegates out the refresh call to whatever needs it, according to the state.
void MossSimulator::Refresh() {
  switch (state) {
    case Growing:
      RefreshGrowing();
      break;
    case Eating:
      RefreshEating();
      break;
    case StoryDisp:
      RefreshStoryDisp();
      break;
    case DbgEntry:
      RefreshDbgEntry();
      break;
    default:
      break;
  }
}

// Creates a texture generator based on what moss the story wants.
void MossSimulator::CreateTexGen() {
  texGen = TextureGenerator();

  // Welcome, traveller, to THE BLOCK 2: ELECTRIC BOOGALOO.
  // You can see THE BLOCK (the first one) above in MossStory::UpdateCurrentVars.
  // THE BLOCK 2 is more excusable than the first, but it still hurts to look at.

  switch (mossStory.GetCurrentMossType()) {
    case MossType::Error:
    default: {
      // If something bad happens, make it super obvious
      constexpr lv_color_t bgCol = LV_COLOR_MAGENTA;
      constexpr lv_color_t squareCol = LV_COLOR_MAKE(0x20, 0x20, 0x20);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Add two layers of offset black squares to create the diagonal repeating pattern
      TextureLayerDataSquare dataSquare = TextureLayerDataSquare(GradientData(0.0, 0.5, squareCol, LV_OPA_100).SetClip(true, true), 120);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, dataSquare.SetShift(-30, -30)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, dataSquare.SetShift(30, 30)));
      break;
    }
    case MossType::Basic: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0xA0, 0x00);
      constexpr lv_color_t whiteSpeckleCol = LV_COLOR_WHITE;
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0, 0x30, 0);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of white speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, whiteSpeckleCol, LV_OPA_0, whiteSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      break;
    }
    case MossType::Blue: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x20, 0x20, 0xFF);
      constexpr lv_color_t whiteSpeckleCol = LV_COLOR_WHITE;
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x10, 0x10, 0x30);
      // Blank blue background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of white speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, whiteSpeckleCol, LV_OPA_0, whiteSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      break;
    }
    case MossType::Digital: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0x50, 0x00);
      constexpr lv_color_t darkGreen = LV_COLOR_MAKE(0x00, 0x40, 0x00);
      constexpr lv_color_t midGreen = LV_COLOR_MAKE(0x00, 0x80, 0x00);
      constexpr lv_color_t lightGreen = LV_COLOR_MAKE(0x00, 0xA0, 0x00);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Add a few layers of different brightness squares
      // Dark green squares in bg. Reusing the same gradient because most data is the same between layers.
      GradientData darkGreenGradient = GradientData(0.0, 0.0, darkGreen, LV_OPA_100).SetClip(false, true);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(darkGreenGradient.SetEndpoints(0.0, 0.8), 80)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(darkGreenGradient.SetEndpoints(0.0, 0.5), 100)));
      // Mid green squares around middle layers. Reusing the same gradient because most data is the same between layers.
      GradientData midGreenGradient = GradientData(0.0, 0.0, midGreen, LV_OPA_80).SetClip(false, true);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(midGreenGradient.SetEndpoints(0.0, 0.5), 145)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(midGreenGradient.SetEndpoints(0.0, 0.25), 200)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(midGreenGradient.SetEndpoints(0.0, 0.25), 200)));
      // Light green squares in foreground. Reusing the same gradient because most data is the same between layers.
      GradientData lightGreenGradient = GradientData(0.0, 0.0, lightGreen, LV_OPA_50).SetClip(false, true);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(lightGreenGradient.SetEndpoints(0.0, 0.2), 100)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(lightGreenGradient.SetEndpoints(0.0, 0.3), 150)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(lightGreenGradient.SetEndpoints(0.0, 0.5), 180)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(lightGreenGradient.SetEndpoints(0.0, 0.25), 200)));
      break;
    }
    case MossType::Gray: {
      // There should be no magenta in this texture. It's still sneaking in somehow.
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x80, 0xA0, 0x80);
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x30, 0x35, 0x30);
      // Blank grayish background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      break;
    }
    case MossType::Dark: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x50, 0x50, 0x50);
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x10, 0x10, 0x10);
      // Blank dark gray background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      break;
    }
    case MossType::DarkGlowyBlue: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x50, 0x60, 0x60);
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x10, 0x10, 0x10);
      constexpr lv_color_t glowCol = LV_COLOR_AQUA;
      // Blank aquaish dark gray background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // Add glowy aqua spots
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(GradientData(0.7, 1.0, glowCol, LV_OPA_0, glowCol, LV_OPA_100), 20)));
      break;
    }
    case MossType::DarkGlowyOrange: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x70, 0x60, 0x50);
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x10, 0x10, 0x10);
      constexpr lv_color_t glowCol = LV_COLOR_ORANGE;
      // Blank orangey dark gray background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // Add glowy orange spots
      // Because orange is a lot duller than aqua, artificially make the spots brighter by making two gradients: high opacity center spots
      //  surrounded by the 'dropoff' gradient.
      // Works well to emulate a 3 point linear gradient, but expensive to calculate since it's 2 Perlin noise layers.
      const GradientData orangeCenterGradient = GradientData(0.8, 1.0, glowCol, LV_OPA_80, glowCol, LV_OPA_100).SetClip(true, false);
      const GradientData orangeOuterGradient = GradientData(0.65, 0.8, glowCol, LV_OPA_0, glowCol, LV_OPA_80).SetClip(true, true);
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(orangeOuterGradient, 20).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(orangeCenterGradient, 20).SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::Wild: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x60, 0xA0, 0x00);
      constexpr lv_color_t whiteSpeckleCol = LV_COLOR_WHITE;
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x00, 0x30, 0x00);
      constexpr lv_color_t darkenOrangeCol = LV_COLOR_MAKE(0x60, 0x30, 0x00);
      // Blank yellowish-green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of white speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, whiteSpeckleCol, LV_OPA_0, whiteSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      const GradientData orangeGradient = GradientData(0.0, 1.0, darkenOrangeCol, LV_OPA_0, darkenOrangeCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(orangeGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(orangeGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      break;
    }
    case MossType::SpanishMossquisition: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x2A, 0x25, 0x12);
      constexpr lv_color_t lightGrayish = LV_COLOR_MAKE(0x96, 0x95, 0x8B);
      // Dark tannish background layer
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Three layers of overlaid stretched moss to make it look sort of like it's hanging down
      TextureLayerDataPerlin edgeLGrad =
        TextureLayerDataPerlin(GradientData(0.4, 0.5, lightGrayish, LV_OPA_0, lightGrayish, LV_OPA_100).SetClip(true, true), 0);
      TextureLayerDataPerlin edgeRGrad =
        TextureLayerDataPerlin(GradientData(0.5, 0.6, lightGrayish, LV_OPA_100, lightGrayish, LV_OPA_0).SetClip(true, true), 0);
      int16_t shiftX = std::rand();
      int16_t shiftY = std::rand();
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, edgeLGrad.SetScale(6, 24).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, edgeRGrad.SetScale(6, 24).SetShift(shiftX, shiftY)));
      shiftX = std::rand();
      shiftY = std::rand();
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, edgeLGrad.SetScale(8, 32).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, edgeRGrad.SetScale(8, 32).SetShift(shiftX, shiftY)));
      shiftX = std::rand();
      shiftY = std::rand();
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, edgeLGrad.SetScale(8, 32).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, edgeRGrad.SetScale(8, 32).SetShift(shiftX, shiftY)));

      break;
    }
    case MossType::Soul: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x49, 0x18, 0x62);   // Purple
      constexpr lv_color_t soulTop = LV_COLOR_MAKE(0x6a, 0x96, 0x0b); // Green
      constexpr lv_color_t soulMid = LV_COLOR_MAKE(0x73, 0x5c, 0x01); // Brown
      constexpr lv_color_t soulBot = LV_COLOR_MAKE(0x77, 0x00, 0xb1); // Purple
      // Three layers of Perlin noise to sort of make a ghostly vertical texture
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      GradientData soulGradient = GradientData(0.7, 1.0, LV_COLOR_BLACK, LV_OPA_60, LV_COLOR_BLACK, LV_OPA_100).SetClip(true, true);
      // Limit shiftX and shiftY harshly to guarantee that auto-correction in SetShift doesn't kick in and mess up the layer alignment.
      const int16_t shiftX = std::rand() & 0x3FFF;
      const int16_t shiftY = std::rand() & 0x3FFF;
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(soulGradient.SetEndpoints(0.7, 1.0).SetColor(soulBot), 0).SetScale(60, 20).SetShift(shiftX, shiftY + 15)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(soulGradient.SetEndpoints(0.65, 1.0).SetColor(soulMid), 0).SetScale(60, 20).SetShift(shiftX, shiftY + 7)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(soulGradient.SetEndpoints(0.55, 1.0).SetColor(soulTop), 0).SetScale(60, 20).SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::Fiery: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0x60, 0x00);
      constexpr lv_color_t whiteSpeckleCol = LV_COLOR_WHITE;
      constexpr lv_color_t flameColorEdge = LV_COLOR_MAKE(0xFF, 0x50, 0);
      constexpr lv_color_t flameColorInner = LV_COLOR_MAKE(0xC0, 0xC0, 0x50);
      constexpr lv_color_t smokeCol = LV_COLOR_BLACK;
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of white speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, whiteSpeckleCol, LV_OPA_0, whiteSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to set it alight
      const GradientData flameGradient =
        GradientData(0.4, 1.0, flameColorEdge, LV_OPA_50, flameColorInner, LV_OPA_100).SetClip(true, false);
      const GradientData flameUpperGradient = GradientData(0.0, 1.0, smokeCol, LV_OPA_0, smokeCol, LV_OPA_40).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(flameGradient, 30)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(flameGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(flameUpperGradient, 20)));
      // Orange and black speckles for fire bits and smoke
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, flameColorEdge, LV_OPA_0, flameColorEdge, LV_OPA_70))));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, smokeCol, LV_OPA_0, smokeCol, LV_OPA_70))));
      break;
    }
    case MossType::RedGlow: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x30, 0x00, 0x00);
      constexpr lv_color_t redSpeckleCol = LV_COLOR_RED;
      constexpr lv_color_t brightenCol = LV_COLOR_MAKE(0xFF, 0x10, 0x10);
      constexpr lv_color_t speckleOverlayStartCol = LV_COLOR_MAKE(0x80, 0, 0);
      constexpr lv_color_t speckleOverlayStopCol = LV_COLOR_MAKE(0xFF, 0x80, 0);
      // Blank dark red background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of light red speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, redSpeckleCol, LV_OPA_0, redSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData brightenGradient = GradientData(0.0, 1.0, brightenCol, LV_OPA_0, brightenCol, LV_OPA_70);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(brightenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(brightenGradient, 10)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, speckleOverlayStartCol, LV_OPA_0, speckleOverlayStopCol, LV_OPA_70))));
      break;
    }
    case MossType::Bread: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0xDE, 0xA4, 0x5A);
      constexpr lv_color_t breadDarkenCol = LV_COLOR_BLACK;
      constexpr lv_color_t breadHoleCol = LV_COLOR_MAKE(0xC3, 0x8D, 0x43);
      constexpr lv_color_t breadCrustCol = LV_COLOR_MAKE(0x66, 0x40, 0x07);
      // Blank tan background with speckles for texture
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.7, 1.0, breadDarkenCol, LV_OPA_0, breadDarkenCol, LV_OPA_10))));
      // Holes in the bread
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.68, 1.0, breadHoleCol, LV_OPA_80, breadHoleCol, LV_OPA_100).SetClip(true, false), 15)));
      // Crust around the edge
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare,
                     TextureLayerDataSquare(GradientData(0.90, 1.0, breadCrustCol, LV_OPA_100).SetClip(true, false), 240).SetShift(0, 0)));
      break;
    }
    case MossType::Carpet: {
      constexpr lv_color_t carpetBaseCol = LV_COLOR_MAKE(0x20, 0x80, 0x20);
      constexpr lv_color_t carpetBaseColDark = LV_COLOR_MAKE(0x16, 0x50, 0x16);
      constexpr lv_color_t carpetEdgeColor = LV_COLOR_MAKE(0x00, 0x50, 0x00);
      constexpr lv_color_t carpetFluffColor = LV_COLOR_MAKE(0x2F, 0x4C, 0x1A);
      constexpr lv_color_t edgeBGColor = LV_COLOR_MAKE(0x36, 0x1A, 0x09);
      // Base fluffy carpet texture
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.3, 1.0, carpetBaseCol, LV_OPA_100, carpetBaseColDark, LV_OPA_100))));
      // Border around the center carpet texture
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare,
                     TextureLayerDataSquare(GradientData(0.917, 1.0, carpetEdgeColor, LV_OPA_100).SetClip(true, false), 0)
                       .SetShift(0, 15)
                       .SetScale(240, 210)));
      // Fluff on top and bottom edges to make it clearer it's a carpet
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle,
                     TextureLayerDataTriangle(GradientData(0.5, 0.5, carpetFluffColor, LV_OPA_100, edgeBGColor, LV_OPA_100), 0)
                       .SetShift(0, 0)
                       .SetScale(15, 15))
          .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min(), 14));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle,
                     TextureLayerDataTriangle(GradientData(0.5, 0.5, edgeBGColor, LV_OPA_100, carpetFluffColor, LV_OPA_100), 0)
                       .SetShift(20, 0)
                       .SetScale(15, 15))
          .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), 226, std::numeric_limits<int16_t>::max()));
      break;
    }
    case MossType::TVStatic: {
      // Pure grayscale noise
      constexpr lv_color_t fromCol = LV_COLOR_WHITE;
      constexpr lv_color_t toCol = LV_COLOR_BLACK;
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Simple, TextureLayerDataSimple({0.0, 1.0, fromCol, LV_OPA_100, toCol, LV_OPA_100})));
      break;
    }
    case MossType::Cultivated: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0x90, 0x00);
      constexpr lv_color_t darkenColor = LV_COLOR_MAKE(0, 0x30, 0);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenColor, LV_OPA_0, darkenColor, LV_OPA_30);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenColor, LV_OPA_0, darkenColor, LV_OPA_40))));
      break;
    }
    case MossType::Mousse: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0xA4, 0x63, 0x46);
      constexpr lv_color_t bgDarkenCol = LV_COLOR_BLACK;
      constexpr lv_color_t holeCol = LV_COLOR_MAKE(0x89, 0x4B, 0x30);
      // Blank tan background with speckles for texture
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.9, 1.0, bgDarkenCol, LV_OPA_0, bgDarkenCol, LV_OPA_10))));
      // Holes in the mousse
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.68, 1.0, holeCol, LV_OPA_50, holeCol, LV_OPA_100).SetClip(true, false), 5)));
      break;
    }
    case MossType::Sunrise: {
      // Not super proud of this one.
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x1A, 0x00, 0x00);
      constexpr lv_color_t purpleSpeckleCol = LV_COLOR_MAKE(0xD0, 0x00, 0xFF);
      constexpr lv_color_t lightenCol = LV_COLOR_MAKE(0xE0, 0x90, 0x30);
      // Blank orange background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of purple speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, purpleSpeckleCol, LV_OPA_0, purpleSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData lightenGradient = GradientData(0.0, 1.0, lightenCol, LV_OPA_0, lightenCol, LV_OPA_40);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(lightenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(lightenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(lightenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(lightenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, lightenCol, LV_OPA_0, lightenCol, LV_OPA_50))));
      break;
    }
    case MossType::Suspicious: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0xA0, 0x00);
      constexpr lv_color_t whiteSpeckleCol = LV_COLOR_WHITE;
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0, 0x30, 0);
      constexpr lv_color_t sus1Col = LV_COLOR_RED;
      constexpr lv_color_t sus2Col = LV_COLOR_YELLOW;
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of white speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, whiteSpeckleCol, LV_OPA_0, whiteSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 15)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      // Red and yellow speckles to make it 'suspicious'
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.83, 1.0, sus1Col, LV_OPA_50, sus1Col, LV_OPA_70).SetClip(true, false), 10)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.0, 0.17, sus2Col, LV_OPA_70, sus2Col, LV_OPA_50).SetClip(false, true), 10)
                       .SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::TreeMoss: {
      constexpr lv_color_t barkDark = LV_COLOR_MAKE(0x57, 0x28, 0x00);
      constexpr lv_color_t barkLight = LV_COLOR_MAKE(0x7B, 0x4D, 0x25);
      constexpr lv_color_t darkMossColor = LV_COLOR_MAKE(0x00, 0x50, 0x00);
      constexpr lv_color_t lightMossColor = LV_COLOR_MAKE(0x10, 0x90, 0x10);
      // Background bark
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(barkDark, LV_OPA_100)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.4, 0.6, barkLight, LV_OPA_100, barkLight, LV_OPA_100).SetClip(true, true), 0)
                       .SetScale(20, 80)));
      // Overlaid moss
      // Uses the same color and gradient several times
      GradientData darkenGradient = GradientData(0.4, 1.0, LV_COLOR_BLACK, LV_OPA_30, LV_COLOR_BLACK, LV_OPA_100).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient.SetColor(lightMossColor), 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient.SetColor(darkMossColor), 15)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient.SetColor(darkMossColor), 5)));
      // Speckling to give the illusion of detail
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, LV_COLOR_BLACK, LV_OPA_0, lightMossColor, LV_OPA_50))));
      break;
    }
    case MossType::MossMoss: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0xFF, 0x00);
      constexpr lv_color_t whiteSpeckleCol = LV_COLOR_WHITE;
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0, 0x15, 0);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of white speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, whiteSpeckleCol, LV_OPA_0, whiteSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      break;
    }
    case MossType::QueerPride: {
      constexpr lv_color_t redCol = LV_COLOR_RED;
      constexpr lv_color_t orangeCol = LV_COLOR_ORANGE;
      constexpr lv_color_t yellowCol = LV_COLOR_YELLOW;
      constexpr lv_color_t greenCol = LV_COLOR_GREEN;
      constexpr lv_color_t blueCol = LV_COLOR_BLUE;
      constexpr lv_color_t purpleCol = LV_COLOR_PURPLE;
      constexpr uint16_t prideScale = 60;
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      // The values here (and for all Queer* mosses) were obtained by generating a lot of Perlin noise and observing its output,
      //  then partitioning the results such that each segment has equal amounts of area.
      // Side note: the distribution of values generated by Perlin noise looks really weird.
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.0, 0.352, redCol, LV_OPA_100).SetClip(false, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.352, 0.436, orangeCol, LV_OPA_100).SetClip(true, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.436, 0.5, yellowCol, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.5, 0.564, greenCol, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.564, 0.647, blueCol, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.647, 1.0, purpleCol, LV_OPA_100).SetClip(true, false), prideScale).SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::QueerTrans: {
      constexpr lv_color_t transWhite = LV_COLOR_WHITE;
      constexpr lv_color_t transBlue = LV_COLOR_MAKE(0x22, 0xCE, 0xF6);
      constexpr lv_color_t transPink = LV_COLOR_MAKE(0xF0, 0xA9, 0xB9);
      constexpr uint16_t prideScale = 60;
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.0, 0.463, transBlue, LV_OPA_100).SetClip(false, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.463, 0.626, transPink, LV_OPA_100).SetClip(true, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.626, 1.0, transWhite, LV_OPA_100).SetClip(true, false), prideScale)
                       .SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::QueerLesbian: {
      constexpr lv_color_t lesbianOrange = LV_COLOR_MAKE(0xD5, 0x2D, 0x00);
      constexpr lv_color_t lesbianLightOrange = LV_COLOR_MAKE(0xFF, 0x9B, 0x57);
      constexpr lv_color_t lesbianWhite = LV_COLOR_WHITE;
      constexpr lv_color_t lesbianLightPurplish = LV_COLOR_MAKE(0xD3, 0x62, 0xA4);
      constexpr lv_color_t lesbianPurplish = LV_COLOR_MAKE(0xA3, 0x02, 0x62);
      constexpr uint16_t prideScale = 60;
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.0, 0.373, lesbianOrange, LV_OPA_100).SetClip(false, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.373, 0.463, lesbianLightOrange, LV_OPA_100).SetClip(true, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.463, 0.538, lesbianWhite, LV_OPA_100).SetClip(true, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.538, 0.626, lesbianLightPurplish, LV_OPA_100).SetClip(true, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.626, 1.0, lesbianPurplish, LV_OPA_100).SetClip(true, false), prideScale)
                       .SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::QueerGay: {
      constexpr lv_color_t gayGreen3 = LV_COLOR_MAKE(0x07, 0x8D, 0x70);
      constexpr lv_color_t gayGreen2 = LV_COLOR_MAKE(0x26, 0xCE, 0xAA);
      constexpr lv_color_t gayGreen1 = LV_COLOR_MAKE(0x98, 0xE8, 0xC1);
      constexpr lv_color_t gayWhite = LV_COLOR_WHITE;
      constexpr lv_color_t gayBlue1 = LV_COLOR_MAKE(0x7B, 0xAD, 0xE2);
      constexpr lv_color_t gayBlue2 = LV_COLOR_MAKE(0x50, 0x49, 0xCC);
      constexpr lv_color_t gayBlue3 = LV_COLOR_MAKE(0x3D, 0x1A, 0x78);
      constexpr uint16_t prideScale = 60;
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      // 7 stripes hurts performance-wise... But implementing proper multi-point gradients would hurt more.
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.0, 0.337, gayGreen3, LV_OPA_100).SetClip(false, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.337, 0.419, gayGreen2, LV_OPA_100).SetClip(true, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.419, 0.474, gayGreen1, LV_OPA_100).SetClip(true, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.474, 0.529, gayWhite, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.529, 0.584, gayBlue1, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.584, 0.663, gayBlue2, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.663, 1.0, gayBlue3, LV_OPA_100).SetClip(true, false), prideScale).SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::QueerBi: {
      constexpr lv_color_t biMagenta = LV_COLOR_MAKE(0xD7, 0x00, 0x71);
      constexpr lv_color_t biPurple = LV_COLOR_MAKE(0xAA, 0x5B, 0xA4);
      constexpr lv_color_t biBlue = LV_COLOR_MAKE(0x00, 0x35, 0xAA);
      constexpr uint16_t prideScale = 60;
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      // Thresholds in this flag moss were adjusted because it looked a bit strange with accurate 2:1:2 color ratios.
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.0, 0.46, biMagenta, LV_OPA_100).SetClip(false, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.46, 0.54, biPurple, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.54, 1.0, biBlue, LV_OPA_100).SetClip(true, false), prideScale).SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::QueerPan: {
      constexpr lv_color_t panMagenta = LV_COLOR_MAKE(0xFF, 0x21, 0x8C);
      constexpr lv_color_t panYellow = LV_COLOR_MAKE(0xFF, 0xD8, 0x00);
      constexpr lv_color_t panCyan = LV_COLOR_MAKE(0x21, 0xB1, 0xFF); // hey, panCyan rhymes!
      constexpr uint16_t prideScale = 60;
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.0, 0.438, panMagenta, LV_OPA_100).SetClip(false, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.438, 0.564, panYellow, LV_OPA_100).SetClip(true, true), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.564, 1.0, panCyan, LV_OPA_100).SetClip(true, false), prideScale).SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::QueerEnby: {
      constexpr lv_color_t enbyYellow = LV_COLOR_MAKE(0xFC, 0xF4, 0x34);
      constexpr lv_color_t enbyWhite = LV_COLOR_WHITE;
      constexpr lv_color_t enbyPurple = LV_COLOR_MAKE(0x9C, 0x59, 0xD1);
      constexpr lv_color_t enbyDarkGray = LV_COLOR_MAKE(0x2C, 0x2C, 0x2C);
      constexpr uint16_t prideScale = 60;
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.0, 0.4, enbyYellow, LV_OPA_100).SetClip(false, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.4, 0.5, enbyWhite, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.5, 0.6, enbyPurple, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.6, 1.0, enbyDarkGray, LV_OPA_100).SetClip(true, false), prideScale)
                       .SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::QueerAce: {
      constexpr lv_color_t aceBlackish = LV_COLOR_MAKE(0x2C, 0x2C, 0x2C);
      constexpr lv_color_t aceGray = LV_COLOR_MAKE(0xA4, 0xA4, 0xA4);
      constexpr lv_color_t aceWhite = LV_COLOR_WHITE;
      constexpr lv_color_t acePurple = LV_COLOR_MAKE(0x7F, 0x00, 0x68);
      constexpr uint16_t prideScale = 60;
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.0, 0.4, aceBlackish, LV_OPA_100).SetClip(false, false), prideScale)
                       .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.4, 0.5, aceGray, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.5, 0.6, aceWhite, LV_OPA_100).SetClip(true, true), prideScale).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.6, 1.0, acePurple, LV_OPA_100).SetClip(true, false), prideScale).SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::Grass: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0xA0, 0x00);
      constexpr lv_color_t bgMottleCol = LV_COLOR_MAKE(0x00, 0x30, 0x00);
      constexpr lv_color_t grassStrandBG = LV_COLOR_MAKE(0x15, 0x60, 0x00);
      constexpr lv_color_t grassStrandFG = LV_COLOR_MAKE(0x20, 0x90, 0x00);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, bgMottleCol, LV_OPA_0, bgMottleCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      // Strands of grass on top
      const GradientData strandBGGradient = GradientData(0.45, 0.55, grassStrandBG, LV_OPA_100).SetClip(true, true);
      const GradientData strandFGGradient = GradientData(0.45, 0.55, grassStrandFG, LV_OPA_100).SetClip(true, true);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(strandBGGradient, 0).SetScale(4, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(strandFGGradient, 0).SetScale(3, 30)));
      // Fake high resolution perlin layer that's not actually perlin. Likely unnoticeable but meh.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, bgMottleCol, LV_OPA_0, bgMottleCol, LV_OPA_70).SetClip(true, false))));
      break;
    }
    case MossType::Big: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x05, 0x10, 0x00);
      constexpr lv_color_t brightenColor = LV_COLOR_MAKE(0x05, 0x90, 0);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      const int16_t shiftX = std::rand() & 0x3FFF;
      const int16_t shiftY = std::rand() & 0x3FFF;
      GradientData darkenGradient = GradientData(0.0, 1.0, brightenColor, LV_OPA_0, brightenColor, LV_OPA_50).SetClip(true, true);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 80)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 15).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient.SetEndpoints(0.5, 1.0), 15).SetShift(shiftX, shiftY - 4)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient.SetEndpoints(0.6, 1.0), 15).SetShift(shiftX, shiftY - 8)));
      // High resolution noise to give it a nicer texture
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Simple,
        TextureLayerDataSimple(GradientData(0.6, 1.0, brightenColor, LV_OPA_0, brightenColor, LV_OPA_40).SetClip(true, false))));
      break;
    }
    case MossType::Zombie: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x30, 0x80, 0x30);
      constexpr lv_color_t whiteSpeckleCol = LV_COLOR_WHITE;
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x20, 0x20, 0x10);
      // Blank dark green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of white speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, whiteSpeckleCol, LV_OPA_0, whiteSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 15)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      break;
    }
    case MossType::Moon: {
      constexpr lv_color_t bgColor = LV_COLOR_MAKE(0x50, 0x60, 0x80);
      constexpr lv_color_t darkenColor = LV_COLOR_MAKE(0x10, 0x20, 0x30);
      constexpr lv_color_t moonColor = LV_COLOR_MAKE(0xFF, 0xF9, 0xB0);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgColor, LV_OPA_100)));
      // Two offset layers of Perlin noise to make a sort of wiggly "moon" look
      // Accidental misuse of blending creates a haze to the left of squiggles. I'm calling it a feature because it looks cool.
      const int16_t shiftX = std::rand() & 0x3FFF;
      const int16_t shiftY = std::rand() & 0x3FFF;
      GradientData wigglyMoonGrad = GradientData(0.0, 0.4, moonColor, LV_OPA_50, moonColor, LV_OPA_100).SetClip(false, true);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(wigglyMoonGrad, 50).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(wigglyMoonGrad.SetColors(bgColor, bgColor), 50).SetShift(shiftX - 10, shiftY)));
      // Add a big moon in the center. Has an outline to fade out nearby wigglyMoon squiggles, then place the big moon just about centered.
      const GradientData centerMoonOutlineGrad =
        GradientData(1 / std::numbers::sqrt2_v<float> / 2, 4.f / 3.f / std::numbers::sqrt2_v<float>, bgColor, LV_OPA_100, bgColor, LV_OPA_0)
          .SetClip(false, true);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(centerMoonOutlineGrad, 240).SetShift(-10, 0)));
      GradientData centerMoon =
        GradientData(0, 1 / std::numbers::sqrt2_v<float> / 2, LV_COLOR_BLACK, LV_OPA_100, LV_COLOR_BLACK, LV_OPA_100).SetClip(false, true);
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(centerMoon.SetColors(moonColor, moonColor), 240).SetShift(0, 0)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(centerMoon.SetColors(bgColor, bgColor), 240).SetShift(-20, 0)));
      // Layers of Perlin to give it a nice texture
      // Uses the same color and gradient several times
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenColor, LV_OPA_0, darkenColor, LV_OPA_50).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 15)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // Fake high resolution perlin layer that's not actually perlin. Likely unnoticeable but meh.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, darkenColor, LV_OPA_0, darkenColor, LV_OPA_70).SetClip(true, false))));
      break;
    }
    case MossType::Aurora: {
      constexpr lv_color_t graySky = LV_COLOR_MAKE(0x20, 0x20, 0x20);
      constexpr lv_color_t starColor = LV_COLOR_WHITE;
      constexpr lv_color_t aurora1 = LV_COLOR_MAKE(0x83, 0x2D, 0x9B);
      constexpr lv_color_t aurora2Start = LV_COLOR_MAKE(0x6A, 0x9C, 0x1B);
      constexpr lv_color_t aurora2End = LV_COLOR_MAKE(0x4E, 0xA4, 0x00);
      constexpr lv_color_t aurora3Start = LV_COLOR_MAKE(0x78, 0x4D, 0x98);
      constexpr lv_color_t aurora3End = LV_COLOR_MAKE(0xA9, 0x26, 0xCD);
      // Blank sky
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(graySky, LV_OPA_100)));
      // Aurora borealis!
      GradientData auroraMainGrad = GradientData(0.46, 1.0, LV_COLOR_BLACK, LV_OPA_20, LV_COLOR_BLACK, LV_OPA_80).SetClip(true, false);
      GradientData auroraSubGrad = GradientData(0.6, 1.0, LV_COLOR_BLACK, LV_OPA_40, LV_COLOR_BLACK, LV_OPA_90).SetClip(true, false);
      // First layer of aurora: dark purple
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(auroraMainGrad.SetColors(aurora1, aurora1), 0).SetScale(10, 120)));
      // Second layer of aurora: green. Creates object used in layer 3 as well.
      int16_t shiftX = std::rand();
      int16_t shiftY = std::rand();
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(auroraMainGrad.SetColors(aurora2Start, aurora2End), 0).SetScale(10, 120).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(auroraSubGrad.SetColors(aurora2Start, aurora2End), 0).SetScale(10, 120).SetShift(shiftX, shiftY + 60)));
      // Square layer to fade out green and dark purple auroras toward the top.
      // Uses a square quadruple the edge length of the screen and positions it so the viewed area is at the top middle of the square.
      const GradientData auroraBlocker = GradientData(0.5, 1.0, graySky, LV_OPA_0, graySky, LV_OPA_100).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(auroraBlocker, 960).SetShift(-360, 0)));
      // Third layer of aurora: purple-pink
      shiftX = std::rand();
      shiftY = std::rand();
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(auroraMainGrad.SetColors(aurora3Start, aurora3End), 0).SetScale(10, 180).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(auroraSubGrad.SetColors(aurora3Start, aurora3End), 0).SetScale(10, 180).SetShift(shiftX, shiftY + 90)));
      // Stars
      const TextureLayerDataSimple whiteSimpleData(GradientData(0.97, 1.0, starColor, LV_OPA_0, starColor, LV_OPA_60).SetClip(true, false));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Simple, whiteSimpleData));
      break;
    }
    case MossType::Sickened: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x50, 0x60, 0x80);
      constexpr lv_color_t whiteSpeckleCol = LV_COLOR_WHITE;
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x10, 0x30, 0x30);
      constexpr lv_color_t sick1 = LV_COLOR_RED;
      constexpr lv_color_t sick2 = LV_COLOR_GREEN;
      constexpr lv_color_t sick3 = LV_COLOR_BLUE;
      constexpr lv_color_t sick4 = LV_COLOR_PURPLE;
      constexpr lv_color_t sick5 = LV_COLOR_YELLOW;
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of white speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, whiteSpeckleCol, LV_OPA_0, whiteSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 30)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      // Wibbly wobbly "Good Lorde i am Greatley Sickened" effect
      constexpr lv_opa_t sickOpa = LV_OPA_50;
      const GradientData layer1 = GradientData(0.0, 0.4, sick1, sickOpa, sick2, sickOpa).SetClip(false, true);
      const GradientData layer2 = GradientData(0.4, 0.5, sick2, sickOpa, sick3, sickOpa).SetClip(true, true);
      const GradientData layer3 = GradientData(0.5, 0.6, sick3, sickOpa, sick4, sickOpa).SetClip(true, true);
      const GradientData layer4 = GradientData(0.6, 1.0, sick4, sickOpa, sick5, sickOpa).SetClip(true, false);
      const int16_t shiftX = std::rand();
      const int16_t shiftY = std::rand();
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(layer1, 50).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(layer2, 50).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(layer3, 50).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(layer4, 50).SetShift(shiftX, shiftY)));
      break;
    }
    case MossType::Lichen: {
      constexpr lv_color_t grayBG = LV_COLOR_MAKE(0x78, 0x75, 0x70);
      constexpr lv_color_t grayBG2 = LV_COLOR_MAKE(0x63, 0x60, 0x5A);
      constexpr lv_color_t lichen1 = LV_COLOR_MAKE(0xBA, 0x63, 0x1C);
      constexpr lv_color_t lichen1Edge = LV_COLOR_MAKE(0xAB, 0x40, 0x08);
      constexpr lv_color_t lichen2 = LV_COLOR_MAKE(0xC6, 0xA6, 0x5B);
      constexpr lv_color_t lichen2Edge = LV_COLOR_MAKE(0xCA, 0xAF, 0x54);
      constexpr lv_color_t darkenColor = LV_COLOR_MAKE(0x40, 0x40, 0x40);
      // Gray stony background
      // texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(grayBG, LV_OPA_100)));
      const GradientData bgGrad = GradientData(0.0, 1.0, grayBG, LV_OPA_100, grayBG2, LV_OPA_100);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(bgGrad, 30)));
      const GradientData bgDarkenGrad = GradientData(0.0, 1.0, darkenColor, LV_OPA_0, darkenColor, LV_OPA_40);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(bgDarkenGrad)));
      // Two lichen colors
      const GradientData lichen1Grad = GradientData(0.55, 1.0, lichen1Edge, LV_OPA_100, lichen1, LV_OPA_100).SetClip(true, false);
      const GradientData lichen2Grad = GradientData(0.65, 1.0, lichen2Edge, LV_OPA_100, lichen2, LV_OPA_100).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(lichen1Grad, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(lichen2Grad, 30)));
      // Some simple noise darkening to give it texture
      const GradientData fluffyDarkenGrad = GradientData(0.0, 1.0, darkenColor, LV_OPA_0, darkenColor, LV_OPA_30);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(fluffyDarkenGrad)));
      break;
    }
    case MossType::Glass: {
      constexpr lv_color_t windowBase = LV_COLOR_MAKE(0xBF, 0xFF, 0xFF);
      constexpr lv_color_t border = LV_COLOR_MAKE(0x6D, 0x30, 0x00);
      constexpr lv_color_t highlight = LV_COLOR_WHITE;
      // Blue base color
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(windowBase, LV_OPA_100)));
      // Highlight triangles
      const GradientData highlightGrad = GradientData(0.0, 0.10, highlight, LV_OPA_100).SetClip(false, true);
      const GradientData highlightInvGrad = GradientData(0.0, 0.10, windowBase, LV_OPA_100).SetClip(false, true);
      int16_t shiftX = -40;
      int16_t shiftY = -90;
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle, TextureLayerDataTriangle(highlightGrad, 120).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle, TextureLayerDataTriangle(highlightInvGrad, 120).SetShift(shiftX + 3, shiftY + 3)));
      shiftX += 7;
      shiftY += 7;
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle, TextureLayerDataTriangle(highlightGrad, 120).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle, TextureLayerDataTriangle(highlightInvGrad, 120).SetShift(shiftX + 3, shiftY + 3)));
      shiftX += 7;
      shiftY += 7;
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle, TextureLayerDataTriangle(highlightGrad, 120).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle, TextureLayerDataTriangle(highlightInvGrad, 120).SetShift(shiftX + 3, shiftY + 3)));
      // Square border
      // 4px border
      const GradientData windowBorder = GradientData(0.9833, 1.0, border, LV_OPA_100, border, LV_OPA_100).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(windowBorder, 240).SetShift(-120, -120)));
      break;
    }
    case MossType::ShowerCurtain: {
      constexpr lv_color_t baseColor = LV_COLOR_MAKE(0x12, 0xFF, 0xB5);
      constexpr lv_color_t darkColor = LV_COLOR_MAKE(0x06, 0x8B, 0x61);
      constexpr lv_color_t darkerColor = LV_COLOR_MAKE(0x08, 0x53, 0x3B);
      constexpr lv_color_t holeColor = LV_COLOR_MAKE(0x60, 0x60, 0x60);
      // Main shower curtain
      const GradientData curtainBGGrad = GradientData(0.51, 1.0, baseColor, LV_OPA_100, darkColor, LV_OPA_100);
      const GradientData curtainBGDarkenGrad = GradientData(0.53, 1.0, darkColor, LV_OPA_0, darkerColor, LV_OPA_100).SetClip(true, false);
      const uint16_t shiftX = std::rand() & 0x3FFF;
      const uint16_t shiftY = std::rand() & 0x3FFF;
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(curtainBGGrad, 0).SetScale(20, 360).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(curtainBGDarkenGrad, 0).SetScale(20, 360).SetShift(shiftX - 5, shiftY)));
      // Circles at the top to emulate holes
      const GradientData holeGrad = GradientData(0.0, 1 / std::numbers::sqrt2_v<float> / 2, holeColor, LV_OPA_100).SetClip(false, true);
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(holeGrad, 0).SetScale(17, 25).SetShift(0, 0))
          .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min(), 25));
      // Squares going between these holes
      const GradientData poleGrad = GradientData(0.0, 0.35, holeColor, LV_OPA_100).SetClip(false, true);
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(poleGrad, 0).SetScale(34, 21).SetShift(17, 2))
          .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min(), 25));
      break;
    }
    case MossType::Brick: {
      constexpr lv_color_t mortar = LV_COLOR_MAKE(0xD9, 0xC5, 0xAE);
      constexpr lv_color_t brickRed = LV_COLOR_MAKE(0xA4, 0x49, 0x3B);
      constexpr lv_color_t overlayCol = LV_COLOR_BLACK;
      // Base mortar
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(mortar, LV_OPA_100)));
      // Bricks on top of mortar. Using some weird math which isn't quite correct but gives correct results anyway.
      const GradientData brickGrad = GradientData(0.0, 0.45652, brickRed, LV_OPA_100, brickRed, LV_OPA_100).SetClip(false, true);
      const int16_t shiftX = std::rand() & 0x3FFF;
      const int16_t shiftY = std::rand() & 0x3FFF;
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(brickGrad, 0).SetScale(44, 46).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(brickGrad, 0).SetScale(44, 46).SetShift(shiftX + 21, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(brickGrad, 0).SetScale(44, 46).SetShift(shiftX + 24, shiftY + 23)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(brickGrad, 0).SetScale(44, 46).SetShift(shiftX + 45, shiftY + 23)));
      // And some noise to give it :sparkles: texture :sparkles:
      const GradientData fluffyDarkenGrad = GradientData(0.0, 1.0, overlayCol, LV_OPA_0, overlayCol, LV_OPA_20).SetClip(false, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(fluffyDarkenGrad)));
      break;
    }
    case MossType::Stars: {
      constexpr lv_color_t starEdge = LV_COLOR_MAKE(0xFF, 0xF7, 0x98);
      constexpr lv_color_t starCenter = LV_COLOR_WHITE;
      constexpr lv_color_t skyBG = LV_COLOR_MAKE(0x20, 0x20, 0x20);
      // Base gray color
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(skyBG, LV_OPA_100)));
      // Three layers of very large stars, abusing the shape of circle textures
      GradientData bigStarGrad = GradientData(0.97, 1.0, starEdge, LV_OPA_100, starCenter, LV_OPA_100).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bigStarGrad, 0).SetScale(120, 160)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bigStarGrad, 0).SetScale(140, 180)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bigStarGrad, 0).SetScale(150, 190)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bigStarGrad, 0).SetScale(160, 200)));
      // Small stars on top
      const GradientData smallStarGrad = GradientData(0.99, 1.0, starEdge, LV_OPA_100, starCenter, LV_OPA_100).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(smallStarGrad)));
      break;
    }
    case MossType::EntireHouse: {
      // Yes, this was painful to make.
      // No, it's not meant to look good.
      // Yes, I listened to the Celeste OST while making it.
      // This ends the Q&A. Thank you for your questions.
      constexpr lv_color_t skyCol = LV_COLOR_MAKE(0xAA, 0xFF, 0xF9);
      constexpr lv_color_t skyNoiseCol = LV_COLOR_MAKE(0x9C, 0xF1, 0xEB);
      constexpr lv_color_t sunCol = LV_COLOR_MAKE(0xFF, 0xE7, 0x00);
      constexpr lv_color_t grassCol = LV_COLOR_MAKE(0x00, 0xA7, 0x0F);
      constexpr lv_color_t houseWallCol = LV_COLOR_MAKE(0x6C, 0x93, 0xFF);
      constexpr lv_color_t roofCol = LV_COLOR_MAKE(0xA4, 0x49, 0x3B);
      constexpr lv_color_t chimneyCol = LV_COLOR_MAKE(0x79, 0x36, 0x2B);
      constexpr lv_color_t smokeCol = LV_COLOR_MAKE(0x6B, 0x6B, 0x6B);
      constexpr lv_color_t windowCol = LV_COLOR_MAKE(0xBC, 0xFF, 0xFB);
      constexpr lv_color_t doorCol = LV_COLOR_MAKE(0x9C, 0x58, 0x00);
      constexpr lv_color_t doorHandleCol = LV_COLOR_MAKE(0x37, 0x20, 0x00);
      // Mostly using bounds limiting for squares because it's more accurate and easier to work with for art like this.
      // Sky background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(skyCol, LV_OPA_100)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.7, 1.0, skyNoiseCol, LV_OPA_0, skyNoiseCol, LV_OPA_100).SetClip(true, false))));
      // Grass on bottom
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(grassCol, LV_OPA_100))
          .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), 210, std::numeric_limits<int16_t>::max()));
      // Sun at top left
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle,
                     TextureLayerDataCircle(
                       GradientData(0.5, 1 / std::numbers::sqrt2_v<float>, sunCol, LV_OPA_100, sunCol, LV_OPA_0).SetClip(false, true),
                       80)
                       .SetShift(-40, -40))
          .SetBounds(std::numeric_limits<int16_t>::min(), 40, std::numeric_limits<int16_t>::min(), 40));
      // House base
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(houseWallCol, LV_OPA_100)).SetBounds(40, 200, 110, 210));
      // Smoke coming out of the chimney
      TextureLayerDataCircle smokeData =
        TextureLayerDataCircle(GradientData(0.0, 1 / std::numbers::sqrt2_v<float>, smokeCol, LV_OPA_100).SetClip(false, true), 25);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, smokeData.SetShift(157, 23)).SetBounds(157, 181, 23, 47));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, smokeData.SetShift(150, 1)).SetBounds(150, 174, 1, 25));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, smokeData.SetShift(165, -9)).SetBounds(165, 189, -9, 15));
      // House chimney
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(chimneyCol, LV_OPA_100)).SetBounds(157, 180, 44, 95));
      // House roof
      texGen.AddTextureLayer(
        TextureLayer(
          LayerNoise::ShapeTriangle,
          TextureLayerDataTriangle(GradientData(0.0, 0.5, roofCol, LV_OPA_100).SetClip(false, true), 0).SetScale(220, 80).SetShift(10, 30))
          .SetBounds(10, 230, 30, 109));
      // House window
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(windowCol, LV_OPA_100)).SetBounds(70, 110, 140, 180));
      // House door
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(doorCol, LV_OPA_100)).SetBounds(135, 175, 135, 210));
      // House door handle
      texGen.AddTextureLayer(
        TextureLayer(
          LayerNoise::ShapeCircle,
          TextureLayerDataCircle(GradientData(0.5, 1 / std::numbers::sqrt2_v<float>, doorHandleCol, LV_OPA_100).SetClip(false, true), 7)
            .SetShift(167, 170))
          .SetBounds(167, 173, 170, 176));
      break;
    }
    case MossType::GoodLooking: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0xA0, 0x00);
      constexpr lv_color_t abColor = LV_COLOR_MAKE(0x00, 0x20, 0x00);
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x00, 0x30, 0x00);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Set of abs
      // Pectorals, made of two stretched circles and a rectangle
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle,
                     TextureLayerDataCircle(GradientData(0.6, 1 / std::numbers::sqrt2_v<float>, abColor, LV_OPA_100).SetClip(true, true), 0)
                       .SetScale(120, 50)
                       .SetShift(0, 45))
          .SetBounds(10, 230, 70, 94));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle,
                     TextureLayerDataTriangle(GradientData(0.5, 0.5, abColor, LV_OPA_100, bgCol, LV_OPA_100).SetClip(false, false), 0)
                       .SetScale(21, 15)
                       .SetShift(110, 60))
          .SetBounds(111, 129, 65, 74));
      // Pair of (male) nips
      TextureLayerDataCircle nipData =
        TextureLayerDataCircle(GradientData(0.0, 1 / std::numbers::sqrt2_v<float>, abColor, LV_OPA_100).SetClip(false, true), 0)
          .SetScale(20, 10);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, nipData.SetShift(30, 70)).SetBounds(30, 49, 70, 79));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, nipData.SetShift(190, 70)).SetBounds(190, 209, 70, 79));
      // Six-pack of abs
      TextureLayerDataCircle abCurveData = TextureLayerDataCircle(
                                             // very roughly even sized lines in corner and along center
                                             GradientData(0.68, 0.75, abColor, LV_OPA_100).SetClip(true, true),
                                             0)
                                             .SetScale(120, 40);
      TextureLayerDataSquare abStraightData =
        TextureLayerDataSquare(GradientData(0.0, 1.f / 3.f, abColor, LV_OPA_100).SetClip(false, true), 60);
      for (int i = 0; i < 3; i++) {
        const int addY = 40 * i;
        texGen.AddTextureLayer(
          TextureLayer(LayerNoise::ShapeCircle, abCurveData.SetShift(0, 110 + addY)).SetBounds(60, 179, 130 + addY, 149 + addY));
        texGen.AddTextureLayer(
          TextureLayer(LayerNoise::ShapeSquare, abStraightData.SetShift(90, 90 + addY)).SetBounds(118, 122, 110 + addY, 130 + addY));
      }
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      break;
    }
    case MossType::Irradiated: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0x20, 0x00);
      constexpr lv_color_t whiteSpeckleCol = LV_COLOR_WHITE;
      constexpr lv_color_t lightenColor = LV_COLOR_MAKE(0, 0xD0, 0);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Bit of white speckling. Goes on bottom to make it seem intermittent.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, whiteSpeckleCol, LV_OPA_0, whiteSpeckleCol, LV_OPA_60))));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, lightenColor, LV_OPA_0, lightenColor, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 15)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, lightenColor, LV_OPA_0, lightenColor, LV_OPA_70))));
      break;
    }
    case MossType::Monster: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x70, 0x48, 0x50);
      constexpr lv_color_t darkenColor = LV_COLOR_MAKE(0x10, 0x10, 0x10);
      constexpr lv_color_t splotchColor = LV_COLOR_MAKE(0x70, 0x20, 0x00);
      // Blank dark gray background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenColor, LV_OPA_0, darkenColor, LV_OPA_50).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // Add red splotchiness
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::Perlin,
        TextureLayerDataPerlin(GradientData(0.55, 1.0, splotchColor, LV_OPA_0, splotchColor, LV_OPA_100).SetClip(true, false), 20)));
      break;
    }
    case MossType::Frowny: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x40, 0x20, 0x00);
      constexpr lv_color_t faceColor = LV_COLOR_YELLOW;
      constexpr lv_color_t faceElements = LV_COLOR_MAKE(0x20, 0x20, 0x20);
      // Background + face
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Face base
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::ShapeCircle,
        TextureLayerDataCircle(GradientData(0, 1 / std::numbers::sqrt2_v<float>, faceColor, LV_OPA_100).SetClip(false, true), 240)
          .SetShift(0, 0)));
      // Eyes
      TextureLayerDataCircle eyeData =
        TextureLayerDataCircle(
          GradientData(0, 1 / std::numbers::sqrt2_v<float>, faceElements, LV_OPA_100, faceElements, LV_OPA_100).SetClip(false, true),
          0)
          .SetScale(30, 40);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, eyeData.SetShift(65, 60)).SetBounds(65, 94, 60, 99));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, eyeData.SetShift(145, 60)).SetBounds(145, 175, 60, 99));
      // Frown
      texGen.AddTextureLayer(
        TextureLayer(
          LayerNoise::ShapeCircle,
          TextureLayerDataCircle(
            GradientData(0.6, 1 / std::numbers::sqrt2_v<float>, faceElements, LV_OPA_100, faceElements, LV_OPA_100).SetClip(true, true),
            0)
            .SetScale(120, 120)
            .SetShift(60, 140))
          .SetBounds(60, 179, 140, 189));
      break;
    }
    case MossType::Blanket: {
      // I'm not good at making plaid patterns but I think it turned out okay
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x00, 0x74, 0x1D);
      constexpr lv_color_t centerThick = LV_COLOR_MAKE(0x12, 0x2A, 0x00);
      constexpr lv_color_t edgeUnderlay = LV_COLOR_MAKE(0xCF, 0x64, 0x00);
      constexpr lv_color_t edgeThin = LV_COLOR_MAKE(0x00, 0x9B, 0x27);
      // All plaid pattern offsets are based on this base offset
      const int16_t shiftX = std::rand() & 0x3FFF;
      const int16_t shiftY = std::rand() & 0x3FFF;
      // Background color
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Center thick line and its accompanying intersection darkening helper
      GradientData centerThickGrad = GradientData(0.73333, 1.0, centerThick, LV_OPA_50, centerThick, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::ShapeSquare,
        TextureLayerDataSquare(centerThickGrad.SetEndpoints(0.73333, 1.0).SetClip(true, false), 80).SetShift(shiftX + 41, shiftY + 41)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare,
                     TextureLayerDataSquare(centerThickGrad.SetEndpoints(0.0, 0.3).SetClip(false, true), 80).SetShift(shiftX, shiftY)));
      // Additional offset center thick line and its accompanying intersection darkening helper
      GradientData centerOffsetThickGrad = GradientData(0.9, 1.0, centerThick, LV_OPA_50, centerThick, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::ShapeSquare,
        TextureLayerDataSquare(centerOffsetThickGrad.SetEndpoints(0.9, 1.0).SetClip(true, false), 80).SetShift(shiftX + 31, shiftY + 31)));
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::ShapeSquare,
        TextureLayerDataSquare(centerOffsetThickGrad.SetEndpoints(0.0, 0.1).SetClip(false, true), 80).SetShift(shiftX - 9, shiftY - 9)));
      // Two offset squares to make an underlay for the edge thin lines
      // This produces uneven overlaps in the corners, but I'll just call that giving it character (also it's practically invisible)
      const GradientData edgeUnderlayGrad = GradientData(0.85, 1.0, edgeUnderlay, LV_OPA_30, edgeUnderlay, LV_OPA_30).SetClip(true, false);
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(edgeUnderlayGrad, 80).SetShift(shiftX + 11, shiftY + 11)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(edgeUnderlayGrad, 80).SetShift(shiftX + 68, shiftY + 68)));
      // And finally, the edge thin lines
      const GradientData edgeThinGrad = GradientData(0.983333, 1.0, edgeThin, LV_OPA_100, edgeThin, LV_OPA_100).SetClip(true, false);
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(edgeThinGrad, 80).SetShift(shiftX + 8, shiftY + 8)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(edgeThinGrad, 80).SetShift(shiftX + 14, shiftY + 14)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(edgeThinGrad, 80).SetShift(shiftX + 66, shiftY + 66)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare, TextureLayerDataSquare(edgeThinGrad, 80).SetShift(shiftX + 71, shiftY + 71)));
      break;
    }
    case MossType::DarkBubbly: {
      // Expensive to calculate... Looks pretty cool though.
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x50, 0x50, 0x50);
      constexpr lv_color_t darkenColor = LV_COLOR_MAKE(0x10, 0x10, 0x10);
      constexpr lv_color_t bubbleColor = LV_COLOR_MAKE(0x40, 0x60, 0x40);
      constexpr lv_color_t bubbleHighlightColor = LV_COLOR_MAKE(0x40, 0x70, 0x40);
      // Blank green background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      // Uses the same color and gradient several times
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenColor, LV_OPA_0, darkenColor, LV_OPA_50).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      // Fake high resolution perlin layer that's not actually perlin. Likely unnoticeable but meh.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple,
                     TextureLayerDataSimple(GradientData(0.6, 1.0, darkenColor, LV_OPA_0, darkenColor, LV_OPA_70).SetClip(true, false))));
      // Simple frothiness on the bottom. No highlight because it didn't look good.
      int16_t shiftX = std::rand() & 0x3FFF;
      int16_t shiftY = std::rand() & 0x3FFF;
      const GradientData frothGradient = GradientData(0.5, 1.0, bubbleColor, LV_OPA_60, bubbleColor, LV_OPA_0).SetClip(true, false);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(frothGradient, 40).SetShift(shiftX, shiftY)));
      // Bubbles to go on top
      // Bubbles are 1/2 of the grid size, highlights are 1/8 the grid size
      GradientData bubbleGradient = GradientData(1 / std::numbers::sqrt2_v<float> / 8,
                                                 1 / std::numbers::sqrt2_v<float> / 4,
                                                 bubbleColor,
                                                 LV_OPA_30,
                                                 bubbleColor,
                                                 LV_OPA_100)
                                      .SetClip(false, true);
      GradientData bubbleHighlightGradient =
        GradientData(0.0, 1 / std::numbers::sqrt2_v<float> / 16, bubbleHighlightColor, LV_OPA_100).SetClip(false, true);
      shiftX = std::rand() & 0x3FFF;
      shiftY = std::rand() & 0x3FFF;
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bubbleGradient, 240).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bubbleHighlightGradient, 240).SetShift(shiftX - 15, shiftY - 15)));
      shiftX = std::rand() & 0x3FFF;
      shiftY = std::rand() & 0x3FFF;
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bubbleGradient, 180).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bubbleHighlightGradient, 180).SetShift(shiftX - 10, shiftY - 10)));
      shiftX = std::rand() & 0x3FFF;
      shiftY = std::rand() & 0x3FFF;
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bubbleGradient, 140).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bubbleHighlightGradient, 140).SetShift(shiftX - 7, shiftY - 7)));
      shiftX = std::rand() & 0x3FFF;
      shiftY = std::rand() & 0x3FFF;
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bubbleGradient, 100).SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bubbleHighlightGradient, 100).SetShift(shiftX - 7, shiftY - 7)));
      shiftX = std::rand() & 0x3FFF;
      shiftY = std::rand() & 0x3FFF;
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::ShapeCircle,
        TextureLayerDataCircle(bubbleGradient.SetEndpoints(1 / std::numbers::sqrt2_v<float> / 12, 1 / std::numbers::sqrt2_v<float> / 6),
                               120)
          .SetShift(shiftX, shiftY)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle,
                     TextureLayerDataCircle(bubbleHighlightGradient.SetEndpoints(0.0, 1 / std::numbers::sqrt2_v<float> / 24), 120)
                       .SetShift(shiftX - 5, shiftY - 5)));
      break;
    }
    case MossType::Mossta: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x40, 0x20, 0x00);
      constexpr lv_color_t plateCol = LV_COLOR_MAKE(0xC0, 0xC0, 0xB0);
      constexpr lv_color_t mosstaBG = LV_COLOR_MAKE(0x0C, 0x96, 0x00);
      constexpr lv_color_t mosstaStrandsDark = LV_COLOR_MAKE(0x05, 0x44, 0x00);
      constexpr lv_color_t mosstaStrandsLight = LV_COLOR_MAKE(0x26, 0xD1, 0x17);
      constexpr lv_color_t mosstaShadow = LV_COLOR_MAKE(0x20, 0x10, 0x00);
      // Mossta. Will get surrounded by plate and background to make it look like it's on a plate
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(mosstaBG, LV_OPA_100)));
      // The actual mossta strands. Some dark, some light.
      GradientData mosstaGrad = GradientData(0.44, 0.56, LV_COLOR_BLACK, LV_OPA_20, LV_COLOR_BLACK, LV_OPA_20).SetClip(true, true);
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(mosstaGrad.SetColors(mosstaStrandsDark, mosstaStrandsDark), 20)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(mosstaGrad.SetColors(mosstaStrandsDark, mosstaStrandsDark), 20)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(mosstaGrad.SetColors(mosstaStrandsLight, mosstaStrandsLight), 20)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(mosstaGrad.SetColors(mosstaStrandsLight, mosstaStrandsLight), 20)));
      // Add a shadow to the bottom right of the mossta to give it depth. Also makes it look like an Orb.
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle,
                                          TextureLayerDataCircle(GradientData(3.f / 4.f / std::numbers::sqrt2_v<float>,
                                                                              7.f / 8.f / std::numbers::sqrt2_v<float>,
                                                                              mosstaShadow,
                                                                              LV_OPA_0,
                                                                              mosstaShadow,
                                                                              LV_OPA_50)
                                                                   .SetClip(true, false),
                                                                 240)
                                            .SetShift(-20, -20)));
      // Plate
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle,
                                          TextureLayerDataCircle(GradientData(3.f / 4.f / std::numbers::sqrt2_v<float>,
                                                                              7.f / 8.f / std::numbers::sqrt2_v<float>,
                                                                              plateCol,
                                                                              LV_OPA_0,
                                                                              plateCol,
                                                                              LV_OPA_100)
                                                                   .SetClip(true, false),
                                                                 240)
                                            .SetShift(0, 0)));
      // Border of plate
      texGen.AddTextureLayer(TextureLayer(
        LayerNoise::ShapeCircle,
        TextureLayerDataCircle(GradientData(0.8, 1.0, bgCol, LV_OPA_100, bgCol, LV_OPA_100).SetClip(true, false), 240).SetShift(0, 0)));
      break;
    }
    case MossType::Gold: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0xFF, 0xE3, 0x25);
      constexpr lv_color_t darkenCol = LV_COLOR_MAKE(0x7D, 0x38, 0x00);
      constexpr lv_color_t sparkleCol = LV_COLOR_MAKE(0xFF, 0xF7, 0x98);
      // Blank yellow background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Layers of Perlin to give it a nice texture
      const GradientData darkenGradient = GradientData(0.0, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_50);
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 40)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 20)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 10)));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Perlin, TextureLayerDataPerlin(darkenGradient, 5)));
      // High resolution noise on top to give it an even nicer texture.
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.6, 1.0, darkenCol, LV_OPA_0, darkenCol, LV_OPA_70))));
      // Sparkles on top
      // Three layers of large sparkles, abusing the shape of circle textures
      // The endpoint opacities are intentionally in this order. Makes it look exotic.
      GradientData bigStarGrad = GradientData(0, 1.0, sparkleCol, LV_OPA_100, sparkleCol, LV_OPA_20).SetClip(true, false);
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bigStarGrad.SetEndpoints(0.97, 1.0), 0).SetScale(120, 160)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bigStarGrad.SetEndpoints(0.965, 1.0), 0).SetScale(140, 180)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bigStarGrad.SetEndpoints(0.96, 1.0), 0).SetScale(150, 190)));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle, TextureLayerDataCircle(bigStarGrad.SetEndpoints(0.95, 1.0), 0).SetScale(160, 200)));
      // Single pixel sparkles
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.999, 1.0, sparkleCol, LV_OPA_100).SetClip(true, false))));
      break;
    }
    case MossType::AteAllMoss: {
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x20, 0x20, 0x20);
      constexpr lv_color_t trophyBaseCol = LV_COLOR_MAKE(0xF9, 0xF0, 0x98);
      constexpr lv_color_t trophyHilightCol = LV_COLOR_MAKE(0xFF, 0xFF, 0xD0);
      constexpr lv_color_t trophyShadowCol = LV_COLOR_MAKE(0xBB, 0xB5, 0x73);
      constexpr lv_color_t trophyTextCol = LV_COLOR_MAKE(0x10, 0x72, 0x00);
      constexpr lv_color_t barkDark = LV_COLOR_MAKE(0x57, 0x28, 0x00);
      constexpr lv_color_t barkLight = LV_COLOR_MAKE(0x6D, 0x3F, 0x1D);
      // Blank dark gray background
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(bgCol, LV_OPA_100)));
      // Trophy body base
      texGen.AddTextureLayer(
        TextureLayer(
          LayerNoise::ShapeCircle,
          TextureLayerDataCircle(GradientData(0, 1 / std::numbers::sqrt2_v<float>, trophyBaseCol, LV_OPA_100).SetClip(false, true), 0)
            .SetScale(136, 324)
            .SetShift(52, -150))
          .SetBounds(52, 188, 12, 174));
      // Handle bottoms
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle,
                     TextureLayerDataCircle(
                       GradientData(7.f / 8.f / std::numbers::sqrt2_v<float>, 1 / std::numbers::sqrt2_v<float>, trophyBaseCol, LV_OPA_100)
                         .SetClip(true, true),
                       0)
                       .SetScale(108, 224)
                       .SetShift(12, -100))
          .SetBounds(12, 228, 12, 123));
      // Handle tops
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle,
                     TextureLayerDataCircle(
                       GradientData(3.f / 4.f / std::numbers::sqrt2_v<float>, 1 / std::numbers::sqrt2_v<float>, trophyBaseCol, LV_OPA_100)
                         .SetClip(true, true),
                       0)
                       .SetScale(120, 50)
                       .SetShift(0, -22))
          .SetBounds(12, 228, 12, 28));
      // Trophy body highlight
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle,
                     TextureLayerDataCircle(
                       GradientData(0.5, 1 / std::numbers::sqrt2_v<float>, trophyHilightCol, LV_OPA_100, trophyHilightCol, LV_OPA_0)
                         .SetClip(false, true),
                       0)
                       .SetScale(32, 158)
                       .SetShift(76, -67))
          .SetBounds(76, 108, 12, 91));
      // Trophy body shadow
      texGen.AddTextureLayer(TextureLayer(LayerNoise::ShapeCircle,
                                          TextureLayerDataCircle(GradientData(17.f / 20.f / std::numbers::sqrt2_v<float>,
                                                                              19.f / 20.f / std::numbers::sqrt2_v<float>,
                                                                              trophyShadowCol,
                                                                              LV_OPA_0,
                                                                              trophyShadowCol,
                                                                              LV_OPA_100)
                                                                   .SetClip(true, false),
                                                                 0)
                                            .SetScale(171, 224)
                                            .SetShift(30, -49))
                               .SetBounds(70, 171, 125, 174));
      // Clip trophy body shadow
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeCircle,
                     TextureLayerDataCircle(GradientData(1 / std::numbers::sqrt2_v<float>, 1, bgCol, LV_OPA_100).SetClip(true, false), 0)
                       .SetScale(136, 324)
                       .SetShift(52, -150))
          .SetBounds(70, 171, 125, 174));
      // Trophy shaft
      texGen.AddTextureLayer(
        TextureLayer(
          LayerNoise::ShapeSquare,
          TextureLayerDataSquare(GradientData(0.85, 0.95, trophyShadowCol, LV_OPA_100, trophyBaseCol, LV_OPA_100), 240).SetShift(110, 70))
          .SetBounds(110, 130, 172, 208));
      // Trophy base
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(barkDark, LV_OPA_100)).SetBounds(60, 180, 208, 239));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Perlin,
                     TextureLayerDataPerlin(GradientData(0.4, 0.6, barkLight, LV_OPA_100).SetClip(true, true), 0).SetScale(5, 20))
          .SetBounds(60, 180, 208, 239));
      // Letter E
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare,
                     TextureLayerDataSquare(GradientData(0.705, 1.0, trophyTextCol, LV_OPA_100).SetClip(true, false), 0)
                       .SetScale(34, 17)
                       .SetShift(74, 38))
          .SetBounds(73, 94, 35, 74));
      // letter A
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle,
                     TextureLayerDataTriangle(GradientData(0.37, 0.55, trophyTextCol, LV_OPA_100).SetClip(true, true), 0)
                       .SetScale(37, 40)
                       .SetShift(98, 35))
          .SetBounds(98, 135, 35, 74));
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(trophyTextCol, LV_OPA_100)).SetBounds(106, 127, 59, 64));
      // letter T
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare,
                     TextureLayerDataSquare(GradientData(0.857, 1.0, trophyTextCol, LV_OPA_100).SetClip(true, false), 42).SetShift(151, 37))
          .SetBounds(137, 165, 35, 74));
      // character #
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare,
                     TextureLayerDataSquare(GradientData(0.368, 1.0, trophyTextCol, LV_OPA_100).SetClip(true, false), 0)
                       .SetScale(93, 19)
                       .SetShift(63, 101))
          .SetBounds(93, 125, 101, 119));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeSquare,
                     TextureLayerDataSquare(GradientData(0.368, 1.0, trophyTextCol, LV_OPA_100).SetClip(true, false), 0)
                       .SetScale(19, 113)
                       .SetShift(100, 55))
          .SetBounds(100, 118, 91, 129));
      // number 1
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(trophyTextCol, LV_OPA_100)).SetBounds(134, 145, 87, 129));
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::ShapeTriangle,
                     TextureLayerDataTriangle(GradientData(0.5, 0.5, trophyTextCol, LV_OPA_100, trophyBaseCol, LV_OPA_100), 0)
                       .SetScale(44, 13)
                       .SetShift(116, 87))
          .SetBounds(127, 137, 87, 99));
      break;
    }
    case MossType::Egg: {
      constexpr lv_color_t decoCols[] = {
        LV_COLOR_MAKE(0xE0, 0x51, 0x8A), // magenta-ish
        LV_COLOR_MAKE(0xFF, 0xF3, 0x1B), // yellow
        LV_COLOR_MAKE(0x87, 0xFF, 0x1B), // lime
        LV_COLOR_MAKE(0x00, 0xC9, 0x53), // green
        LV_COLOR_MAKE(0x42, 0xBA, 0xD9), // light blue
        LV_COLOR_MAKE(0x14, 0x4C, 0xBA), // relatively dark blue
        LV_COLOR_MAKE(0x65, 0x19, 0xFF), // indigo
        LV_COLOR_MAKE(0xB8, 0x48, 0xD8), // purple
        LV_COLOR_MAKE(0xF6, 0x00, 0x50), // pinkish red
      };
      constexpr int decoColCount = 9;
      // Opacity here is how strongly to overlay the decorative color onto a natural eggshell color
      const lv_color_t eggBgCol = lv_color_mix(decoCols[std::rand() % decoColCount], LV_COLOR_MAKE(0xE0, 0x87, 0x51), LV_OPA_60);
      constexpr lv_color_t dimpleCol = LV_COLOR_BLACK;
      constexpr lv_color_t bgCol = LV_COLOR_MAKE(0x84, 0x6B, 0x39);
      // Blank background for base egg
      texGen.AddTextureLayer(TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(eggBgCol, LV_OPA_100)));
      for (int i = 0; i < 6; i++) {
        const int rowOffset = i * 40;
        switch (std::rand() % 8) {
          default:
            break;
          case 0:
            // Big dots
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::ShapeCircle,
                           TextureLayerDataCircle(
                             GradientData(0.0, 3.f / 4.f / std::numbers::sqrt2_v<float>, decoCols[std::rand() % decoColCount], LV_OPA_100)
                               .SetClip(false, true),
                             40)
                             .SetShift(std::rand(), rowOffset))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset, rowOffset + 39));
            break;
          case 1:
            // Dots and lines
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::ShapeCircle,
                           TextureLayerDataCircle(
                             GradientData(0.0, 2.f / 3.f / std::numbers::sqrt2_v<float>, decoCols[std::rand() % decoColCount], LV_OPA_100)
                               .SetClip(false, true),
                             30)
                             .SetShift(std::rand(), rowOffset + 5))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 5, rowOffset + 34));
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(decoCols[std::rand() % decoColCount], LV_OPA_100))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 2, rowOffset + 8));
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(decoCols[std::rand() % decoColCount], LV_OPA_100))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 32, rowOffset + 38));
            break;
          case 2:
            // Thick line
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(decoCols[std::rand() % decoColCount], LV_OPA_100))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 10, rowOffset + 30));
            break;
          case 3:
            // Thin lines
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(decoCols[std::rand() % decoColCount], LV_OPA_100))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 4, rowOffset + 10));
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(decoCols[std::rand() % decoColCount], LV_OPA_100))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 15, rowOffset + 25));
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::Blank, TextureLayerDataBlank(decoCols[std::rand() % decoColCount], LV_OPA_100))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 30, rowOffset + 36));
            break;
          case 4:
            // Triangle wavy line
            texGen.AddTextureLayer(
              TextureLayer(
                LayerNoise::ShapeTriangle,
                TextureLayerDataTriangle(GradientData(0.4, 0.6, decoCols[std::rand() % decoColCount], LV_OPA_100).SetClip(true, true), 30)
                  .SetShift(std::rand(), rowOffset + 5))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 5, rowOffset + 34));
            break;
          case 5: {
            // 6 point stars
            const int16_t xOffset = std::rand() & 0x3FFF;
            const lv_color_t triangleCol = decoCols[std::rand() % decoColCount];
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::ShapeTriangle,
                           TextureLayerDataTriangle(GradientData(0.0, 0.4, triangleCol, LV_OPA_100).SetClip(false, true), 0)
                             .SetScale(36, 31)
                             .SetShift(xOffset, rowOffset - 3))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 3, rowOffset + 27));
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::ShapeTriangle,
                           TextureLayerDataTriangle(GradientData(0.6, 1.0, triangleCol, LV_OPA_100).SetClip(true, false), 0)
                             .SetScale(36, 31)
                             .SetShift(xOffset + 18, rowOffset + 12))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 12, rowOffset + 36));
            break;
          }
          case 6: {
            // Smooth wavy line
            const int16_t xOffset = std::rand() & 0x3FFF;
            const lv_color_t circleCol = decoCols[std::rand() % decoColCount];
            // Math hard, brain soft. I eyeballed most of the values here after failing to calculate them correctly.
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::ShapeCircle,
                           TextureLayerDataCircle(
                             GradientData(0.74f / std::numbers::sqrt2_v<float>, 0.95f / std::numbers::sqrt2_v<float>, circleCol, LV_OPA_100)
                               .SetClip(true, true),
                             100)
                             .SetShift(xOffset, rowOffset + 3))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 4, rowOffset + 19));
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::ShapeCircle,
                           TextureLayerDataCircle(
                             GradientData(0.74f / std::numbers::sqrt2_v<float>, 0.95f / std::numbers::sqrt2_v<float>, circleCol, LV_OPA_100)
                               .SetClip(true, true),
                             100)
                             .SetShift(xOffset + 50, rowOffset - 63))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 19, rowOffset + 35));
            break;
          }
          case 7:
            // Staggered rhombuses
            const int16_t xOffset = std::rand() & 0x3FFF;
            const lv_color_t triangleCol = decoCols[std::rand() % decoColCount];
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::ShapeTriangle,
                           TextureLayerDataTriangle(GradientData(0.0, 0.333, triangleCol, LV_OPA_100).SetClip(false, true), 0)
                             .SetScale(54, 27)
                             .SetShift(xOffset, rowOffset - 8))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 2, rowOffset + 19));
            texGen.AddTextureLayer(
              TextureLayer(LayerNoise::ShapeTriangle,
                           TextureLayerDataTriangle(GradientData(0.667, 1.0, triangleCol, LV_OPA_100).SetClip(true, false), 0)
                             .SetScale(54, 27)
                             .SetShift(xOffset + 27, rowOffset + 19))
                .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), rowOffset + 19, rowOffset + 38));
            break;
        }
      }
      // Bit of noise to try to emulate the dimpled surface of an egg
      texGen.AddTextureLayer(
        TextureLayer(LayerNoise::Simple, TextureLayerDataSimple(GradientData(0.8, 1.0, dimpleCol, LV_OPA_0, dimpleCol, LV_OPA_20))));
      // Create overall egg shape by overwriting everything left with a flat color
      texGen.AddTextureLayer(
        TextureLayer(
          LayerNoise::ShapeCircle,
          TextureLayerDataCircle(GradientData(1.f / 2.f / std::numbers::sqrt2_v<float>, 1.0, bgCol, LV_OPA_100).SetClip(true, false), 360)
            .SetShift(-60, -30))
          .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), 150, std::numeric_limits<int16_t>::max()));
      texGen.AddTextureLayer(
        TextureLayer(
          LayerNoise::ShapeCircle,
          TextureLayerDataCircle(GradientData(1.f / 2.f / std::numbers::sqrt2_v<float>, 1.0, bgCol, LV_OPA_100).SetClip(true, false), 0)
            .SetScale(360, 600)
            .SetShift(-60, -150))
          .SetBounds(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min(), 150));
      break;
    }
  }
}

// Refresh function for growing state. Just generates the texture in a big spiral, to try and make it less obvious how  slow the
// texture generation can be... At least it's kinda interesting to look at?
// Will refuse to keep drawing blocks if it's taken more than 2/3 of the task refresh time, to prevent resource starvation.
void MossSimulator::RefreshGrowing() {
  // If user is still holding tap from previous state, ignore it until the tap gets released
  PollStillHoldingTap();

  // Similar code also appears in the DrawGrowingTile function, but is used in a different way.
  int32_t loopDiameter = 1;
  while (growingChunk >= loopDiameter * loopDiameter) {
    loopDiameter += 2;
  }

  // Draw enough tiles so each loop takes around TARGET_LOOP_REFRESH_COUNT refreshes to finish
  unsigned int numTilesToDraw = (loopDiameter * loopDiameter - (loopDiameter - 2) * (loopDiameter - 2)) / TARGET_LOOP_REFRESH_COUNT;
  if (numTilesToDraw == 0) {
    numTilesToDraw = 1;
  }
  // Try not to take much more than 3/4 of the default refresh rate time (not using task refresh rate because that's variable here)
  // This loop also gets broken if DrawGrowingTile() finishes and sets the state out of Growing
  const uint32_t refreshStartTime = xTaskGetTickCount();
  for (unsigned int i = 0;
       i < numTilesToDraw && state == Growing && xTaskGetTickCount() - refreshStartTime < LV_DISP_DEF_REFR_PERIOD * 3 / 4;
       i++) {
    DrawGrowingTile();
  }

  // Only do this if still in Growing state
  if (state == Growing) {
    // Set the refresh period to be 5 milliseconds longer than how long it took to refresh the screen, or if that's shorter than
    //  the default refresh rate, just leave it at default.
    // This gives other tasks on the system time to work.
    lv_task_set_period(taskRefresh,
                       std::max(xTaskGetTickCount() - refreshStartTime + pdMS_TO_TICKS(5), (uint32_t) LV_DISP_DEF_REFR_PERIOD));
  }
}

// Draws a single tile in the spiral drawn during growing phase.
void MossSimulator::DrawGrowingTile() {
  if (growingChunk == 0) {
    CreateTexGen();
  }
  // Drawn chunks are hardcoded at 10x10px, but resolution is negotiable (must be square and a multiple of 20 though!)
  if (growingChunk >= (LV_HOR_RES / 10) * (LV_VER_RES / 10)) {
    // Done growing moss, switch to eating. This also breaks out of the for loop in RefreshGrowing().
    SwitchToEating();
    return;
  }

  // Do some stupid logic to find where in the spiral the block should be drawn
  // I found explicit formulas to calculate this when looking around online, but they look hard (and slow for these pretty small sizes).
  // Populates spiralX and spiralY with simple coordinates of the block! So growingChunk=0 gives (0,0), 1 is (0,1), 2 is (1,1), etc.
  // So it's a clockwise spiral which starts by going up.
  int spiralX = 0;
  int spiralY = 0;
  {
    int32_t loopDiameter = 1;
    // comparing to odd squares (1, 9, 25, 49...)
    while (growingChunk >= loopDiameter * loopDiameter) {
      loopDiameter += 2;
    }
    // Now know which loop the value is in
    const int loopChunk = (int) growingChunk - ((loopDiameter - 2) * (loopDiameter - 2));
    // Icky. Works quite well though and is fast.
    // Does not work for loopDiameter of 1 so just ignore that case and leave spiralX and spiralY at 0.
    if (loopDiameter > 1) {
      switch (loopChunk / (loopDiameter - 1)) {
        case 0:
          // Top horizontal row (excluding top left)
          spiralY = loopDiameter / 2;               // Would need to subtract 1 as well but flooring division works favorably here
          spiralX = -(1 * spiralY) + 1 + loopChunk; // Substituting loopDiameter/2 with spiralY
          break;
        case 1:
          // Right column
          spiralX = loopDiameter / 2;
          spiralY = (3 * spiralX) - 1 - loopChunk;
          break;
        case 2:
          // Bottom row
          spiralY = -loopDiameter / 2;
          spiralX = -(5 * spiralY) - 1 - loopChunk;
          break;
        case 3:
          // Left column
          spiralX = -loopDiameter / 2;
          spiralY = (7 * spiralX) + 1 + loopChunk;
          break;
        default:
          break;
      }
    }
  }
  // spiralX and spiralY now have coordinates for where in the growing spiral the next tile should be drawn

  // Try and prevent timing issues caused by writing to a buffer while it's being drawn
  SwitchActiveBuffer();

  // Calculate bounds of and populate buffer, then draw it to the screen
  const lv_coord_t x1 = (spiralX * 10) + (LV_HOR_RES / 2 - 10);
  const lv_coord_t y1 = (LV_VER_RES / 2) - (spiralY * 10);
  const lv_coord_t x2 = x1 + 9;
  const lv_coord_t y2 = y1 + 9;
  texGen.GetBlock(activeBuffer, x1, y1, x2, y2);
  const lv_area_t area = {x1, y1, x2, y2};
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  lvgl.FlushDisplay(&area, activeBuffer);

  growingChunk++;
}

// Refresh function for eating state. Simply keeps checking if user is done eating, then increments story.
void MossSimulator::RefreshEating() {
  // Check if user is still holding tap somehow
  PollStillHoldingTap();
  // If finished eating moss, go to StoryDisp phase
  if (munchTracker.IsFinished()) {
    mossStory.IncrementStory();
    lv_label_set_text_static(storyText, mossStory.GetCurrentStoryText());
    lv_label_set_text_static(sceneText, mossStory.GetCurrentSceneText());
    SwitchToStoryDisp();
  }
}

// Refresh function for storydisp state. Just checks if user is still holding tap.
void MossSimulator::RefreshStoryDisp() {
  PollStillHoldingTap();
}

// Refresh function for DgbSceneSelect state. Just checks if user is still holding tap and moves to StoryDisp state when they let go.
void MossSimulator::RefreshDbgEntry() {
  PollStillHoldingTap();
  if (!stillHoldingTap) {
    if (xTaskGetTickCount() - stillHoldingTapStartTime >= DEBUG_ENTRY_HOLD_TIME) {
      SwitchToDbgSceneSelect();
    } else {
      lv_label_set_text_static(storyText, mossStory.GetCurrentStoryText());
      lv_label_set_text_static(sceneText, mossStory.GetCurrentSceneText());
      SwitchToStoryDisp();
    }
  }
}

// Used during eating phase, draws a bite mark where the user clicked. ONLY draws to the screen, munch tracking is handled elsewhere.
void MossSimulator::DrawBiteMark(lv_coord_t centerX, lv_coord_t centerY) {
  lv_area_t accumArea;

  // Go through each row and accumulate pixels until a pixel that shouldn't be drawn is found, then draw the accumulated pixels
  for (lv_coord_t y = std::max(centerY - MOSS_MUNCH_RADIUS, 0); y <= std::min(centerY + MOSS_MUNCH_RADIUS, LV_VER_RES - 1); y++) {
    accumArea.y1 = y;
    accumArea.y2 = y;
    accumArea.x1 = -1;
    for (lv_coord_t x = std::max(centerX - MOSS_MUNCH_RADIUS, 0); x <= std::min(centerX + MOSS_MUNCH_RADIUS, LV_HOR_RES - 1); x++) {
      if (IsPointInBiteMark(x, y, centerX, centerY)) {
        if (accumArea.x1 == -1) {
          accumArea.x1 = x;
        }
        accumArea.x2 = x;
      } else {
        if (accumArea.x1 != -1) {
          lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
          lvgl.FlushDisplay(&accumArea, activeBuffer);
        }
        accumArea.x1 = -1;
      }
    }
    // Also check at the end of each row if there are any accumulated pixels
    if (accumArea.x1 != -1) {
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
      lvgl.FlushDisplay(&accumArea, activeBuffer);
    }
  }
}

// Set up everything and switch to growing phase. Expects story to already have been incremented.
void MossSimulator::SwitchToGrowing() {
  CreateTexGen();
  lv_task_set_period(taskRefresh, LV_DISP_DEF_REFR_PERIOD);
  // Let LVGL clear the screen for me...
  lv_obj_refresh_style(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
  lv_obj_set_hidden(storyText, true);
  lv_obj_set_hidden(sceneText, true);
  growingChunk = 0;
  stillHoldingTap = true;
  stillHoldingTapStartTime = xTaskGetTickCount();
  remainingTapHoldLeniency = TOUCH_RELEASE_LENIENCY + 1;
  state = Growing;
}

// Set up everything and switch to eating phase.
// The app cannot initialize to Eating state, since this function doesn't initialize some critical variables like the
//  other two SwitchTo* functions do.
void MossSimulator::SwitchToEating() {
  lv_task_set_period(taskRefresh, LV_DISP_DEF_REFR_PERIOD);
  lv_obj_set_hidden(storyText, true);
  lv_obj_set_hidden(sceneText, true);
  SwitchActiveBuffer();
  std::fill_n(activeBuffer, 100, LV_COLOR_BLACK);
  munchTracker.Reset();
  state = Eating;
}

// Set up everything and switch to storydisp phase.
// Does NOT populate the contents of storyText or sceneText, that must be done before this function call.
void MossSimulator::SwitchToStoryDisp() {
  lv_task_set_period(taskRefresh, LV_DISP_DEF_REFR_PERIOD);
  lv_label_set_align(storyText, LV_LABEL_ALIGN_CENTER);
  lv_label_set_align(sceneText, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(storyText, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_align(sceneText, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
  // Let LVGL clear the screen for me...
  lv_obj_refresh_style(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
  lv_obj_set_hidden(storyText, false);
  lv_obj_set_hidden(sceneText, false);
  stillHoldingTap = true;
  stillHoldingTapStartTime = xTaskGetTickCount();
  remainingTapHoldLeniency = TOUCH_RELEASE_LENIENCY + 1;
  state = StoryDisp;
}

// Set up everything and switch to debug scene select phase. Similar to SwitchToStoryDisp.
void MossSimulator::SwitchToDbgSceneSelect() {
  lv_task_set_period(taskRefresh, LV_DISP_DEF_REFR_PERIOD);
  lv_label_set_text_static(sceneText, mossStory.GetCurrentSceneText());
  lv_label_set_text_static(storyText, "Click side button\nto cycle scene.\nTap to continue.");
  lv_label_set_align(storyText, LV_LABEL_ALIGN_CENTER);
  lv_label_set_align(sceneText, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(storyText, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_align(sceneText, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
  // Let LVGL clear the screen for me...
  lv_obj_refresh_style(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STYLE_PROP_ALL);
  lv_obj_set_hidden(storyText, false);
  lv_obj_set_hidden(sceneText, false);
  stillHoldingTap = false;
  state = DbgSceneSelect;
}

// Set up everything and switch to debug entry phase.
void MossSimulator::SwitchToDbgEntry() {
  lv_task_set_period(taskRefresh, LV_DISP_DEF_REFR_PERIOD);
  lv_obj_set_hidden(storyText, true);
  lv_obj_set_hidden(sceneText, true);
  stillHoldingTap = true;
  stillHoldingTapStartTime = xTaskGetTickCount();
  // For debug entry, only give 1 tick of leniency at the beginning so if user is not holding tap then it immediately releases their tap
  remainingTapHoldLeniency = 1;
  state = DbgEntry;
}

// Checks if user is still holding tap for purposes of not allowing accidental inputs when switching state. Has a rejection timeout as well.
// (e.g. user finishes eating moss and then immediately accidentally clicks through storydisp because they were still holding)
void MossSimulator::PollStillHoldingTap() {
  // If user has let go of tap, or still in tap rejection grace period, don't check holding state
  if (stillHoldingTap && xTaskGetTickCount() - stillHoldingTapStartTime > TOUCH_REJECT_TIMEOUT) {
    // Get tap info
    lv_indev_data_t lvglInputData;
    lvgl.GetTouchPadInfo(&lvglInputData);
    if (lvglInputData.state == LV_INDEV_STATE_REL) {
      // Tap has been released, decrease remaining leniency
      // Prevent potential underflow
      if (remainingTapHoldLeniency > 0) {
        remainingTapHoldLeniency--;
      }
    } else {
      // Tap has either continued or been regained, reset leniency
      remainingTapHoldLeniency = TOUCH_RELEASE_LENIENCY + 1;
    }
    // Leniency has run out, set variable to indicate this
    if (remainingTapHoldLeniency == 0) {
      stillHoldingTap = false;
    }
  }
}