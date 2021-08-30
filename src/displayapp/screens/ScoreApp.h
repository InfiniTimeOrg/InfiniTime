#pragma once

#include "Screen.h"
#include "systemtask/SystemTask.h"
#include "../LittleVgl.h"

namespace Pinetime::Applications::Screens {

  class ScoreApp : public Screen {
  public:
  // TODO: Modes (tennis, other sports, magic the gathering/generic card game...)
    // enum class Modes { Normal, Done };

    ScoreApp(DisplayApp* app);

    ~ScoreApp() override;

    void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

  private:
    uint8_t myScore = 0;
    uint8_t yourScore = 0;

    lv_obj_t* _createButton(lv_align_t alignment, uint8_t x, uint8_t y, uint8_t width, uint8_t height, char text[])

    // TODO: Mode button not printed and not used
    lv_obj_t *btnMyScore, *btnYourScore, *btnMyScoreMinus, *btnYourScoreMinus, *btnMode, *btnReset;

    lv_task_t* taskRefresh;
  };
}
