#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include "components/fs/FS.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include "utility/Persistent.h"

using namespace Pinetime::Utility;

namespace Pinetime {
  namespace Controllers {
    class FS;
  }
  namespace Components {
    class LittleVgl;
  }

  namespace Applications {
    namespace Screens {
      struct PaddleState {
        uint16_t score;
      };

      class Paddle : public Screen, public Persistent<PaddleState> {
      public:
        Paddle(Pinetime::Components::LittleVgl& lvgl, Pinetime::Controllers::FS& fs);
        ~Paddle() override;

        void Refresh() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      protected:
        char* GetPersistencePath() override {
          return "persistent/paddle";
        }

        uint32_t GetDataVersion() override {
          return 0;
        }

      private:
        Pinetime::Components::LittleVgl& lvgl;

        const uint8_t ballSize = 16;

        uint16_t paddlePos = 30; // Paddle center

        int16_t ballX = (LV_HOR_RES - ballSize) / 2;
        int16_t ballY = (LV_VER_RES - ballSize) / 2;

        int8_t dx = 2; // Velocity of the ball in the x_coordinate
        int8_t dy = 3; // Velocity of the ball in the y_coordinate

        lv_obj_t* points;
        lv_obj_t* paddle;
        lv_obj_t* ball;
        lv_obj_t* background;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Paddle> {
      static constexpr Apps app = Apps::Paddle;
      static constexpr const char* icon = Screens::Symbols::paddle;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Paddle(controllers.lvgl, controllers.filesystem);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
