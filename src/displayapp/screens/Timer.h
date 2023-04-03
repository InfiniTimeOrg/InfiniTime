#pragma once

#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "systemtask/SystemTask.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/widgets/Counter.h"
#include <lvgl/lvgl.h>

#include "components/timer/TimerController.h"

namespace Pinetime::Applications::Screens {
  class Timer : public Screen {
  public:
    Timer(Controllers::TimerController& timerController,
          System::SystemTask& systemTask,
          Controllers::MotorController& motorController);
    ~Timer() override;
    void Refresh() override;
    void Reset();
    void ToggleRunning();
    void ButtonPressed(lv_obj_t* obj);
    void ButtonReleased(lv_obj_t* obj);
    void ButtonShortClicked(lv_obj_t* obj);
    bool OnButtonPushed() override;
    bool OnTouchEvent(TouchEvents event) override;
    void MaskReset();
    void SetAlerting();
    void SnoozeAlert();
    void StopAlerting();

  private:
    void ShowTimerRunning();
    void ShowTimerStopped();
    void ShowAlertingButtons();
    void ShowAlerting();
    void ShowSnoozed();
    void UpdateMask();

    Controllers::TimerController& timerController;
    System::SystemTask& systemTask;
    Controllers::MotorController& motorController;

    lv_obj_t* btnPlayPause;
    lv_obj_t* txtPlayPause;
    lv_obj_t* btnStop;
    lv_obj_t* txtStop;
    lv_obj_t* btnSnooze;
    lv_obj_t* txtSnooze;

    lv_task_t* taskStopAlert = nullptr;
    lv_obj_t* btnObjectMask;
    lv_obj_t* highlightObjectMask;
    lv_objmask_mask_t* btnMask;
    lv_objmask_mask_t* highlightMask;

    lv_task_t* taskRefresh;
    Widgets::Counter hourCounter = Widgets::Counter(0, 23, jetbrains_mono_76);
    Widgets::Counter minuteCounter = Widgets::Counter(0, 59, jetbrains_mono_76);

    std::chrono::milliseconds alertTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::minutes(10));

    bool buttonPressing = false;
    lv_coord_t maskPosition = 0;
    TickType_t pressTime = 0;
  };
}
