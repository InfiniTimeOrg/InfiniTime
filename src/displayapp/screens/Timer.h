#pragma once

#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "systemtask/SystemTask.h"
#include "displayapp/LittleVgl.h"

#include "components/timer/TimerController.h"

namespace Pinetime::Applications::Screens {
  class Timer : public Screen {
  public:
    enum class Modes { Normal, Done };

    Timer(DisplayApp* app, Controllers::TimerController& timerController);
    ~Timer() override;
    void Refresh() override;
    void SetDone();
    void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

  private:
    void CreateButtons();
    bool running;
    uint8_t secondsToSet = 0;
    uint8_t minutesToSet = 0;
    Controllers::TimerController& timerController;
    lv_obj_t* time;
    lv_obj_t* msecTime;
    lv_obj_t* btnPlayPause;
    lv_obj_t* txtPlayPause;
    lv_obj_t* btnMinutesUp;
    lv_obj_t* btnMinutesDown;
    lv_obj_t* btnSecondsUp;
    lv_obj_t* btnSecondsDown;
    lv_obj_t* txtMUp;
    lv_obj_t* txtMDown;
    lv_obj_t* txtSUp;
    lv_obj_t* txtSDown;
    lv_task_t* taskRefresh;
  };
}
