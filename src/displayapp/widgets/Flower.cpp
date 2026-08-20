#include "displayapp/widgets/Flower.h"
#include <cmath>

using namespace Pinetime::Applications::Widgets;

Flower::Flower() {
}

void Flower::Create(lv_obj_t* parent) {
  container = lv_obj_create(parent, nullptr);
  lv_obj_set_size(container, CONTAINER_WIDTH, CONTAINER_HEIGHT);
  lv_obj_set_style_local_radius(container, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  stem = lv_line_create(container, nullptr);
  lv_obj_set_style_local_line_color(stem, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(STEM_COLOR));
  lv_obj_set_style_local_line_width(stem, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LINE_WIDTH);
  lv_line_set_y_invert(stem, true);
  lv_obj_align(stem, container, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  petals = lv_line_create(container, nullptr);
  lv_obj_set_style_local_line_width(petals, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LINE_WIDTH);
  lv_obj_set_style_local_line_rounded(petals, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LINE_WIDTH);
  lv_line_set_y_invert(petals, true);
  lv_obj_align(petals, container, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
}

void Flower::Update(uint16_t seed, int stage) {
  uint32_t petalColor = PETAL_COLORS[COLOR_INDEX(seed)];
  int petalLength = MIN_PETAL_LENGTH + 2 * SIZE_INDEX(seed);
  int maxStemHeight = MIN_STEM_HEIGHT + 8 * SIZE_INDEX(seed);
  int numPetals = MIN_PETALS + PETAL_INDEX(seed);
  int angleOffsIndex = ANGLE_OFFS_INDEX(seed);
  int stemHeight = HEIGHT_FROM_STAGE(maxStemHeight, stage);

  lv_obj_set_hidden(stem, stage < 1);
  if (stage >= 1) {
    stemPoints[0].x = 0;
    stemPoints[0].y = 0;
    stemPoints[1].x = 0;
    stemPoints[1].y = stemHeight;
    lv_line_set_points(stem, stemPoints, 2);
    lv_obj_realign(stem);
  }

  lv_obj_set_hidden(petals, stage < 2);
  lv_obj_set_style_local_line_color(petals, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(petalColor));
  if (stage == 2) {
    int xOffs = 1;
    petalPoints[0].x = 0 + xOffs;
    petalPoints[0].y = stemHeight;
    petalPoints[1].x = -1 + xOffs;
    petalPoints[1].y = 1 + stemHeight;
    petalPoints[2].x = 0 + xOffs;
    petalPoints[2].y = petalLength / 2 + stemHeight;
    petalPoints[3].x = 1 + xOffs;
    petalPoints[3].y = 1 + stemHeight;
    petalPoints[4].x = 0 + xOffs;
    petalPoints[4].y = stemHeight;
    lv_line_set_points(petals, petalPoints, 5);
    lv_obj_realign(petals);
  } else if (stage == 3) {
    int xOffs = 2;
    petalPoints[0].x = 0 + xOffs;
    petalPoints[0].y = stemHeight;
    petalPoints[1].x = -2 + xOffs;
    petalPoints[1].y = 1 + stemHeight;
    petalPoints[2].x = 0 + xOffs;
    petalPoints[2].y = petalLength + stemHeight;
    petalPoints[3].x = 2 + xOffs;
    petalPoints[3].y = 1 + stemHeight;
    petalPoints[4].x = 0 + xOffs;
    petalPoints[4].y = stemHeight;
    lv_line_set_points(petals, petalPoints, 5);
    lv_obj_realign(petals);
  } else if (stage == 4) {
    int xOffs = petalLength;
    float angleStep = 6.283185f / (numPetals * 2);
    float angleOffs = (angleStep * 0.25f) * angleOffsIndex;
    for (int k = 0; k < numPetals; k++) {
      int k2 = k * 2;
      petalPoints[k2].x = petalLength * sin((k2 * angleStep) + angleOffs) + xOffs;
      petalPoints[k2].y = stemHeight + petalLength * cos((k2 * angleStep) + angleOffs);
      petalPoints[k2 + 1].x = xOffs;
      petalPoints[k2 + 1].y = stemHeight;
    }
    petalPoints[numPetals * 2].x = petalPoints[0].x;
    petalPoints[numPetals * 2].y = petalPoints[0].y;
    lv_line_set_points(petals, petalPoints, (numPetals * 2) + 1);
    lv_obj_realign(petals);
  } else if (stage == 5) {
    int xOffs = petalLength / 2;
    float angleStep = (6.283185f / ((numPetals * 2) - 1)) * 0.25f;
    float angleOffs = 6.283185f * 3.0f / 8.0f;
    for (int k = 0; k < numPetals; k++) {
      int k2 = k * 2;
      petalPoints[k2].x = petalLength * sin((k2 * angleStep) + angleOffs) + xOffs;
      petalPoints[k2].y = stemHeight + petalLength * cos((k2 * angleStep) + angleOffs);
      petalPoints[k2 + 1].x = xOffs;
      petalPoints[k2 + 1].y = stemHeight;
    }
    petalPoints[numPetals * 2].x = petalPoints[0].x;
    petalPoints[numPetals * 2].y = petalPoints[0].y;
    lv_line_set_points(petals, petalPoints, (numPetals * 2) + 1);
    lv_obj_realign(petals);
  }
}
