#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

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

        Calculator();

        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

      private:
        lv_obj_t* buttonMatrix {};
        lv_obj_t* valueLabel {};
        lv_obj_t* resultLabel {};

        void Eval();
        void ResetInput();
        void HandleInput();
        void UpdateValueLabel();
        void UpdateResultLabel() const;
        void UpdateOperation() const;

        // change this if you want to change the number of decimals
        static constexpr uint8_t N_DECIMALS = 3;
        // this is the constant default offset
        static constexpr int64_t FIXED_POINT_OFFSET = powi(10, N_DECIMALS);
        // this is the current offset, may wary after pressing '.'
        int64_t offset = FIXED_POINT_OFFSET;

        // the screen can show 12 chars
        // but two are needed for '.' and '-'
        static constexpr uint8_t MAX_DIGITS = 12;
        static constexpr int64_t MAX_VALUE = powi(10, MAX_DIGITS) - 1;
        // this is assumed in the multiplication overflow!
        static constexpr int64_t MIN_VALUE = -MAX_VALUE;

        int64_t value = 0;
        int64_t result = 0;
        char operation = ' ';
        bool equalSignPressed = false;

        enum Error {
          TooLarge,
          ZeroDivision,
          None,
        };

        Error error = Error::None;
      };
    }

    template <>
    struct AppTraits<Apps::Calculator> {
      static constexpr Apps app = Apps::Calculator;
      static constexpr const char* icon = Screens::Symbols::calculator;

      static Screens::Screen* Create(AppControllers& /* controllers */) {
        return new Screens::Calculator();
      };
    };
  }
}
