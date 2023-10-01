#pragma once

#include "components/motion/MotionController.h"
#include "components/motor/MotorController.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "systemtask/SystemTask.h"

#include <lvgl/lvgl.h>

#include <array>
#include <random>

#define ROLL_HYSTERESIS 10

namespace Pinetime::Applications::Screens {
  class Dice : public Screen {
  public:
    Dice(Controllers::MotionController& motion, Controllers::MotorController& motor, Controllers::Settings& settings);
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

    std::mt19937 gen;

    std::array<lv_color_t, 3> resultColors = {LV_COLOR_YELLOW, LV_COLOR_MAGENTA, LV_COLOR_AQUA};
    uint8_t currentColorIndex;
    void NextColor();

    Widgets::Counter nCounter = Widgets::Counter(1, 9, jetbrains_mono_42);
    Widgets::Counter dCounter = Widgets::Counter(2, 99, jetbrains_mono_42);

    bool openingRoll = true;
    unsigned int rollHysteresis = 0;

    Controllers::MotorController& motor;
    Controllers::MotionController& motion;
    Controllers::Settings& settings;
  };
}
