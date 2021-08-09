#pragma once

#include "systemtask/SystemTask.h"
#include "components/motor/MotorController.h"
#include <controller/ble_ll.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Dice : public Screen {
      public:
        Dice(DisplayApp* app,
             Controllers::MotorController& motorController,
             System::SystemTask& systemTask);
        ~Dice() override;

        void numDiceIncBtnEventHandler(lv_event_t event);
        void numDiceDecBtnEventHandler(lv_event_t event);
        void diceSizeIncBtnEventHandler(lv_event_t event);
        void diceSizeDecBtnEventHandler(lv_event_t event);
        void rollBtnEventHandler(lv_event_t event);

      private:
        Pinetime::System::SystemTask& systemTask;
        Controllers::MotorController& motorController;

        static constexpr uint8_t diceSizes[] = {2, 3, 4, 6, 8, 10, 12, 20, 100};

        uint8_t numDice = 1;
        uint8_t diceSizeIndex = 3;

        void createScreen();
        void rollDice();

        lv_obj_t *resultCont, *resultTotalLabel, *resultIndRollsLabel;
        lv_obj_t *configCont, *rollBtn;
        lv_obj_t* numDiceIncBtn;
        lv_obj_t *numDiceLabelCont, *numDiceLabel;
        lv_obj_t* numDiceDecBtn;
        lv_obj_t* diceSizeIncBtn;
        lv_obj_t *diceSizeLabelCont, *diceSizeLabel;
        lv_obj_t* diceSizeDecBtn;
      };
    }
  }
}
