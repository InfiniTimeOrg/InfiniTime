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
        lv_obj_t* container;
        lv_obj_t* stem;
        lv_obj_t* petals;
        lv_point_t stemPoints[2];
        lv_point_t petalPoints[29];
      };
    }
  }
}
