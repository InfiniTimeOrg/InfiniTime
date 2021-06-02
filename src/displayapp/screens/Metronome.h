#pragma once

#include "systemtask/SystemTask.h"
#include "components/motor/MotorController.h"

#include <array>

namespace Pinetime::Applications::Screens {

  class Metronome : public Screen {
  public:
    Metronome(DisplayApp* app, Controllers::MotorController& motorController, System::SystemTask& systemTask);
    ~Metronome() override;
    bool Refresh() override;
    void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
    enum class States { Running, Stopped };

  private:
    bool running;
    States currentState;
    TickType_t startTime;
    Controllers::MotorController& motorController;
    System::SystemTask& systemTask;
    uint16_t bpm = 120;

    lv_obj_t *bpmText, *bpmLabel;
    lv_obj_t *btnPlayPause, *txtPlayPause;
    lv_obj_t *bpmUp, *bpmUpTxt, *bpmDown, *bpmDownTxt;
  };
}
