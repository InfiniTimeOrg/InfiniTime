#pragma once
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class Flower {
      public:
        Flower();
        void Create(lv_obj_t* parent);
        void Update(uint16_t seed, int stage);

        lv_obj_t* GetObject() {
          return container;
        }

      private:
        static const int LINE_WIDTH = 3;
        static const int CONTAINER_HEIGHT = 115;
        static const int MIN_PETAL_LENGTH = 12;
        static const int MAX_PETAL_LENGTH = 27;
        static const int MIN_PETALS = 7;
        static constexpr int MAX_STEM_HEIGHT = CONTAINER_HEIGHT - MAX_PETAL_LENGTH;
        static constexpr int MIN_STEM_HEIGHT = MAX_PETAL_LENGTH + 5;
        static const uint32_t STEM_COLOR = 0x135416;
        static const int NUM_PETAL_COLORS = 8;
        static constexpr int CONTAINER_WIDTH = MAX_PETAL_LENGTH * 2 + LINE_WIDTH * 2;

        static constexpr uint16_t PETAL_INDEX(uint16_t seed) {
          return seed & 0x07UL;
        }

        static constexpr uint16_t SIZE_INDEX(uint16_t seed) {
          return (seed >> 3) & 0x07UL;
        }

        static constexpr uint16_t ANGLE_OFFS_INDEX(uint16_t seed) {
          return (seed >> 6) & 0x07UL;
        }

        static constexpr uint16_t COLOR_INDEX(uint16_t seed) {
          return (seed >> 9) % NUM_PETAL_COLORS;
        }

        static constexpr int HEIGHT_FROM_STAGE(int max, int stage) {
          return stage > 5 ? max * 3 / 4 : max * stage / 5;
        }

        static constexpr uint32_t PETAL_COLORS[NUM_PETAL_COLORS] =
          {0xf43838, 0xf48829, 0xf9f44d, 0x56efe2, 0x6f7afc, 0x8f68f9, 0xdb49fc, 0xfc83b5};

        lv_obj_t* container;
        lv_obj_t* stem;
        lv_obj_t* petals;
        lv_point_t stemPoints[2];
        lv_point_t petalPoints[29];
      };
    }
  }
}
