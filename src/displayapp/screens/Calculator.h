#pragma once

#include "Screen.h"

namespace {
  int64_t constexpr powi(int64_t base, uint8_t exponent) {
    int64_t value = 1;
    while (exponent) {
      value *= base;
      exponent--;
    }
    return value;
  }
}

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

        // change this if you want to change the number of decimals
        static constexpr uint8_t N_DECIMALS = 4;
        // this is the constant default offset
        static constexpr int64_t FIXED_POINT_OFFSET = powi(10, N_DECIMALS);
        // this is the current offset, may wary after pressing '.'
        int64_t offset = FIXED_POINT_OFFSET;

        int64_t value = 0;
        int64_t result = 0;
        // this has length 2 because it must be a string
        // because we also use it as the buffer for the operationLabel
        // the second char is always \0
        // we only care about the first char
        char operation[2] {" "};
      };
    }
  }
}
