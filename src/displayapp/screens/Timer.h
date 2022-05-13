#pragma once

#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "systemtask/SystemTask.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/widgets/Counter.h"

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
    void SetTimerRunning();
    void SetTimerStopped();
    Controllers::TimerController& timerController;
    lv_obj_t* time;
    lv_obj_t* msecTime;
    lv_obj_t* btnPlayPause;
    lv_obj_t* txtPlayPause;
    lv_task_t* taskRefresh;
    Widgets::Counter minuteCounter = Widgets::Counter(0, 59);
    Widgets::Counter secondCounter = Widgets::Counter(0, 59);
  };
}
