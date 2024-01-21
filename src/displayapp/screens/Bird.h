#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include "displayapp/screens/Screen.h"
#include "Symbols.h"
#include "systemtask/SystemTask.h"

#define BIRD_X        110
#define BIRD_SIZE     20
#define CACTUS_HEIGHT 160
#define CACTUS_WIDTH  30
#define CACTUS_GAP    80

namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }

  namespace Applications {
    namespace Screens {

      class Bird : public Screen {
      public:
        Bird();
        ~Bird() override;

        void Refresh() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        void GameOver();
        void MovePipe();

        bool is_stopped = true;
        bool is_ascending = false;

        uint8_t cactus_x = 240;
        uint8_t cactus_y_offset = 40;

        int8_t accel = 0;
        uint8_t pos = 120;
        uint16_t score = 0;

        lv_obj_t *info, *points, *bird, *cactus_top, *cactus_bottom;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Bird> {
      static constexpr Apps app = Apps::Bird;
      static constexpr const char* icon = Screens::Symbols::dove;

      static Screens::Screen* Create(AppControllers& /*controllers*/) {
        return new Screens::Bird();
      };
    };
  }
}
