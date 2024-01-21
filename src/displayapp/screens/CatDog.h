#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include <displayapp/Controllers.h>
#include "Symbols.h"
#define HORIZON   18000
#define MIN_POWER 200
#define MAX_POWER 500

namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }

  namespace Applications {
    namespace Screens {

      class CatDog : public Screen {
      public:
        CatDog();
        ~CatDog() override;

        void Refresh() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        enum class State : uint8_t {
          Idle,
          Stopped,
          Prompt,
          WaitUser,
          ComputerThrowing,
          UserThrowing,
          BoneFlying,
          CatHit,
          DogHit,
          Missed,
          Result
        };
        State state = State::Idle;

        int16_t throwPower = MIN_POWER;
        int16_t PCTargetThrowPower;

        // distance unit: 0.01 pixel
        int8_t wind = 0; // -4 to 4, a total of 9 levels
        int16_t boneX = 0;
        int16_t boneY = HORIZON;
        int16_t boneVX = 0;
        int16_t boneVY = 0;

        uint8_t catLifePoint = 0;
        uint8_t dogLifePoint = 0;
        uint8_t animationTime = 10;
        bool isPressing = false;
        bool isDogsTurn = true;
        bool isGameStopped = true;

        lv_obj_t *info, *powerBar, *windBarBG, *windBar, *windTxt, *catLifeBar, *dogLifeBar;
        lv_obj_t *bone, *cat, *dog;
        lv_task_t* taskRefresh;

        void Throw(int16_t speed);
        void RandomWind();
        void NextState();
        void UpdateLife();
        void handleDamage(uint8_t distance);
      };
    }

    template <>
    struct AppTraits<Apps::CatDog> {
      static constexpr Apps app = Apps::CatDog;
      static constexpr const char* icon = Screens::Symbols::bone;

      static Screens::Screen* Create(AppControllers& /*controllers*/) {
        return new Screens::CatDog();
      };
    };
  }
}
