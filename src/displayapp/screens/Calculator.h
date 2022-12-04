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

          static constexpr FIXED_POINT_OFFSET = 1000;

          long int value = 0;
          long int result = 0;
          char operation = ' ';
      };
    }
  }
}
