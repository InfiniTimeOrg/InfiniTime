#pragma once

#include "Screen.h"
#include "../LittleVgl.h"
#include "components/motor/MotorController.h"
#include "systemtask/SystemTask.h"
#include <lvgl/src/lv_core/lv_obj.h>



namespace Pinetime::Applications::Screens {

  class ScoreApp : public Screen {
  public:
    // TODO(toitoinou): Modes (tennis, other sports, magic the gathering/generic card game...)
    // enum class Modes { Normal, Done };

    explicit ScoreApp(DisplayApp* app, Controllers::MotorController& motorController);

    ~ScoreApp() override;

    void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

  private:
    static constexpr uint16_t displayWidth = 240;
    static constexpr uint16_t displayHeight = 240;

    static constexpr uint16_t scoreWidth = (displayWidth * 60) / 100;
    static constexpr uint16_t scoreHeight = displayHeight / 2;

    static constexpr uint16_t minusWidth = displayWidth - scoreWidth;
    static constexpr uint16_t minusHeight = displayHeight / 4;

    static constexpr uint16_t modeWidth = displayWidth - scoreWidth;
    static constexpr uint16_t modeHeight = displayHeight / 4;

    Controllers::MotorController& motorController;

    struct widget_t {
      lv_obj_t* button;
      lv_obj_t* label;
    };
    typedef enum : uint8_t {
      SIMPLE_COUNTER = 0,
      BADMINTON = 1,
      TENNIS = 2,
      RESET = 3,
    } mode_t;

    struct score_t {
      uint8_t points;
      uint8_t oldPoints;
      uint8_t game; // only for tennis
      uint8_t sets;
      uint8_t oldSets;
    };

    score_t score[2] = {0, 0, 0, 0};
    mode_t mode = SIMPLE_COUNTER;

    widget_t createButton(lv_align_t alignment, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const char text[]);
    
    // TODO(toitoinou): instead of a general type use something specific to prevent out of bounds errors
    void scoreMainButtonAction(uint8_t scoreId);
    void scoreSecondaryButtonAction(uint8_t scoreId);

    widget_t score1Wdg, score2Wdg;
    widget_t score1SecondaryWdg, score2SecondaryWdg;
    widget_t sets1Wdg, sets2Wdg; // TODO(toitoinou) use a label only instead of a widget/button
    lv_obj_t *modeDropdownWdg;

    lv_task_t* taskRefresh;
  };
}
