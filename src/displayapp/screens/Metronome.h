#pragma once

#include "systemtask/SystemTask.h"
#include "components/motor/MotorController.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Metronome : public Screen {
      public:
        Metronome(Controllers::MotorController& motorController, System::SystemTask& systemTask);
        ~Metronome() override;
        void Refresh() override;
        void OnEvent(lv_obj_t* obj, lv_event_t event);
        bool OnTouchEvent(TouchEvents event) override;

      private:
        TickType_t startTime = 0;
        TickType_t tappedTime = 0;
        Controllers::MotorController& motorController;
        System::SystemTask& systemTask;
        int16_t bpm = 120;
        uint8_t bpb = 4;
        uint8_t counter = 1;

        bool metronomeStarted = false;
        bool allowExit = false;

        lv_obj_t *bpmArc, *bpmTap, *bpmValue;
        lv_obj_t *bpbDropdown, *currentBpbText;
        lv_obj_t* playPause;
        lv_obj_t* lblPlayPause;

        lv_task_t* taskRefresh;
      };
    }
  }
}
