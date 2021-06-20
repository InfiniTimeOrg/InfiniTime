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
    bool OnTouchEvent(TouchEvents event) override;
    void OnEvent(lv_obj_t* obj, lv_event_t event);
    enum class States { Running, Stopped };

  private:
    bool running;
    States currentState;
    TickType_t startTime;
    TickType_t tappedTime = 0;
    Controllers::MotorController& motorController;
    System::SystemTask& systemTask;
    uint16_t bpm = 120;
    uint8_t bpb = 4;
    uint8_t counter = 1;

    lv_obj_t *bpmArc, *bpmTap, *bpmValue, *bpmLegend;
    lv_obj_t *bpbDropdown, *bpbLegend;
    lv_obj_t *playPause, *playPauseLabel;
  };
}
