#pragma once

#include "Screen.h"


namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Calculator : public Screen {
        public:
          ~Calculator() override;

          Calculator(DisplayApp* app);

          void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

        private:
          lv_obj_t* buttonMatrix;
          lv_obj_t* valueLabel;
          lv_obj_t* resultLabel;
          lv_obj_t* operationLabel;

          void Eval();

          int value = 0;
          int result = 0;
          char operation = ' ';
      };
    }
  }
}
