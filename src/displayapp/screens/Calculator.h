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
        void UpdateValueLabel();
        void UpdateResultLabel();

        // offset is the current offset for new digits
        // standard is FIXED_POINT_OFFSET for 3 decimal places
        // after typing a . this gets divided by 10 with each input
        static constexpr int64_t FIXED_POINT_OFFSET = 1000;
        int64_t offset = FIXED_POINT_OFFSET;

        int64_t value = 0;
        int64_t result = 0;
        char operation[2] {" "};
      };
    }
  }
}
