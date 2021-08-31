#pragma once

#include "Screen.h"
#include "systemtask/SystemTask.h"
#include "../LittleVgl.h"
#include <lvgl/src/lv_core/lv_obj.h>

namespace Pinetime::Applications::Screens {

  class ScoreApp : public Screen {
  public:
    // TODO: Modes (tennis, other sports, magic the gathering/generic card game...)
    // enum class Modes { Normal, Done };

    ScoreApp(DisplayApp* app);

    ~ScoreApp() override;

    void Refresh() override;

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

    uint8_t myScore = 0;
    uint8_t yourScore = 0;

    widget_t _createButton(lv_align_t alignment, uint8_t x, uint8_t y, uint8_t width, uint8_t height, const char text[]);

    // TODO: Mode button not printed and not used
    widget_t btnMyScore, btnYourScore, btnMyScoreMinus, btnYourScoreMinus, btnMode, btnReset;

    lv_task_t* taskRefresh;
  };
}
