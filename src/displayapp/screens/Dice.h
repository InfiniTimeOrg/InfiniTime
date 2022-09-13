#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "components/datetime/DateTimeController.h"
#include <lvgl/lvgl.h>

namespace Pinetime::Applications::Screens {
  class Dice : public Screen {
  public:
    Dice(DisplayApp* app, Controllers::DateTime& dateTime);
    ~Dice() override;
    void Roll();
    void NextColor();

  private:
    lv_obj_t* btnRoll;
    lv_obj_t* btnRollLabel;
    lv_obj_t* resultLabel;

    lv_color_t resultColors[3] = {LV_COLOR_YELLOW, LV_COLOR_MAGENTA, LV_COLOR_AQUA};
    uint8_t resultColorsLength = sizeof(resultColors) / sizeof(resultColors[0]);
    uint8_t currentColorIndex;

    uint8_t rollResult;

    Widgets::Counter sidesCounter = Widgets::Counter(2, 99, jetbrains_mono_42);
  };
}
