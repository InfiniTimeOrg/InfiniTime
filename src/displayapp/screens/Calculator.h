
#pragma once


#include "Screen.h"
#include "components/motor/MotorController.h"
#include <array>
#include <string>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Calculator : public Screen {
      public:
        ~Calculator() override;

        Calculator(DisplayApp* app, Controllers::MotorController& motorController);

        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

      private:
        lv_obj_t *result, *returnButton, *buttonMatrix;

        char text[31];
        uint8_t position = 0;

        void eval();

        Controllers::MotorController& motorController;
      };

    }
  }
}
