#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/Counter.h"
#include "components/datetime/DateTimeController.h"
#include <lvgl/lvgl.h>
#include <array>

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

    std::array<lv_color_t, 3> resultColors = {LV_COLOR_YELLOW, LV_COLOR_MAGENTA, LV_COLOR_AQUA};
    uint8_t currentColorIndex;

    Widgets::Counter sidesCounter = Widgets::Counter(2, 99, jetbrains_mono_42);
  };
}
