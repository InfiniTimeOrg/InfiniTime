#pragma once

#include "Screen.h"
#include "../LittleVgl.h"
#include "systemtask/SystemTask.h"
#include <lvgl/src/lv_core/lv_obj.h>



namespace Pinetime::Applications::Screens {

  class ScoreApp : public Screen {
  public:
    // TODO(toitoinou): Modes (tennis, other sports, magic the gathering/generic card game...)
    // enum class Modes { Normal, Done };

    explicit ScoreApp(DisplayApp* app);

    ~ScoreApp() override;

    void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

  private:
    static constexpr uint16_t displayWidth = 240;
    static constexpr uint16_t displayHeight = 240;

    static constexpr uint16_t scoreWidth = (displayWidth * 60) / 100;
    static constexpr uint16_t scoreHeight = displayHeight / 2;

    static constexpr uint16_t minusWidth = displayWidth - scoreWidth;
    static constexpr uint16_t minusHeight = displayHeight / 4;

    static constexpr uint16_t resetWidth = displayWidth - scoreWidth;
    static constexpr uint16_t resetHeight = displayHeight / 4;

    struct widget_t {
      lv_obj_t* button;
      lv_obj_t* label;
    };
    typedef enum {
      SIMPLE_COUNTER,
      BADMINTON,
      TENNIS,
    } mode_t;

    struct score_t {
      uint8_t points;
      uint8_t game; // only for tennis
      uint8_t set;
    };

    score_t score1 = {0, 0, 0};
    score_t score2 = {0, 0, 0};
    mode_t mode = SIMPLE_COUNTER;

    widget_t createButton(lv_align_t alignment, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const char text[]);
    
    ScoreApp::score_t scoreMainButtonAction(ScoreApp::score_t score);
    ScoreApp::score_t scoreSecondaryButtonAction(ScoreApp::score_t score);

    // TODO(toitoinou): Mode button not printed and not used
    widget_t score1Wdg, score2Wdg, score1SecondaryWdg, score2SecondaryWdg, btnMode, resetWdg;

    lv_task_t* taskRefresh;
  };
}
