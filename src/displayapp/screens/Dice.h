#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

#include <array>
#include <random>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Dice : public Screen {
      public:
        Dice(Controllers::MotionController& motionController,
             Controllers::MotorController& motorController,
             Controllers::Settings& settingsController);
        ~Dice() override;
        void Roll();
        void Refresh() override;

      private:
        lv_obj_t* btnRoll;
        lv_obj_t* btnRollLabel;
        lv_obj_t* resultTotalLabel;
        lv_obj_t* resultIndividualLabel;
        lv_task_t* refreshTask;
        bool enableShakeForDice = false;

        // std::mt19937 keeps 624 words of state and needs roughly 2.5 KB of
        // stack to seed from a seed_seq, on a display task that has 3.2 KB in
        // total. A dice roll does not need that much generator.
        class Rng {
        public:
          using result_type = uint32_t;

          static constexpr result_type min() {
            return 1;
          }

          static constexpr result_type max() {
            return UINT32_MAX;
          }

          void Seed(result_type value) {
            // Zero is the one state xorshift cannot leave again.
            state = (value != 0) ? value : 0x9e3779b9;
          }

          result_type operator()() {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return state;
          }

        private:
          result_type state = 0x9e3779b9;
        };

        Rng gen;

        std::array<lv_color_t, 3> resultColors = {LV_COLOR_YELLOW, LV_COLOR_MAGENTA, LV_COLOR_AQUA};
        uint8_t currentColorIndex;
        void NextColor();

        Widgets::Counter nCounter = Widgets::Counter(1, 9, jetbrains_mono_42);
        Widgets::Counter dCounter = Widgets::Counter(2, 99, jetbrains_mono_42);

        bool openingRoll = true;
        uint8_t currentRollHysteresis = 0;
        static constexpr uint8_t rollHysteresis = 10;

        Controllers::MotorController& motorController;
        Controllers::MotionController& motionController;
        Controllers::Settings& settingsController;
      };
    }

    template <>
    struct AppTraits<Apps::Dice> {
      static constexpr Apps app = Apps::Dice;
      static constexpr const char* icon = Screens::Symbols::dice;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Dice(controllers.motionController, controllers.motorController, controllers.settingsController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
