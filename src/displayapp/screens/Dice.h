#pragma once

#include "components/motion/MotionController.h"
#include "components/motor/MotorController.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "systemtask/SystemTask.h"

#include <lvgl/lvgl.h>

#include <array>
#include <random>

namespace Pinetime::Applications::Screens {
  class Dice : public Screen {
  public:
    Dice(Controllers::MotionController& motion, Controllers::MotorController& motor);
    ~Dice() override;
    void Roll();

  private:
    lv_obj_t* btnRoll;
    lv_obj_t* btnRollLabel;
    lv_obj_t* resultTotalLabel;
    lv_obj_t* resultIndividualLabel;

    std::mt19937 gen;

    std::array<lv_color_t, 3> resultColors = {LV_COLOR_YELLOW, LV_COLOR_MAGENTA, LV_COLOR_AQUA};
    uint8_t currentColorIndex;
    void NextColor();

    Widgets::Counter nCounter = Widgets::Counter(1, 9, jetbrains_mono_42);
    Widgets::Counter dCounter = Widgets::Counter(2, 99, jetbrains_mono_42);

    bool openingRoll = true;

    Controllers::MotorController& motor;
  };
}
