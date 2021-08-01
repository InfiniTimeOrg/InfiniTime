#pragma once

#include <cstdint>
#include "../TouchEvents.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;
    namespace Screens {

      template <class T> class DirtyValue {
      public:
        DirtyValue() = default; // Use NSDMI
        explicit DirtyValue(T const& v) : value {v} {
        } // Use MIL and const-lvalue-ref
        bool IsUpdated() {
          if (this->isUpdated) {
            this->isUpdated = false;
            return true;
          }
          return false;
        }
        T const& Get() {
          this->isUpdated = false;
          return value;
        } // never expose a non-const lvalue-ref
        DirtyValue& operator=(const T& other) {
          if (this->value != other) {
            this->value = other;
            this->isUpdated = true;
          }
          return *this;
        }

      private:
        T value {};            // NSDMI - default initialise type
        bool isUpdated {true}; // NSDMI - use brace initilisation
      };

      class Screen {
      public:
        explicit Screen(DisplayApp* app) : app {app} {
        }
        virtual ~Screen() = default;

        /**
         * Most of the time, apps only react to events (touch events, for example).
         * In this case you don't need to do anything in this method.
         *
         * For example, InfiniPaint does nothing in Refresh().
         * But, if you want to update your display periodically, draw an animation...
         * you cannot do it in a touch event handler because these handlers are not
         * called if the user does not touch the screen.
         *
         * That's why Refresh() is there: update the display periodically.
         *
         * @return false if the app can be closed, true if it must continue to run
         **/
        virtual bool Refresh() = 0;

        /** @return false if the button hasn't been handled by the app, true if it has been handled */
        virtual bool OnButtonPushed() {
          return false;
        }

        /** @return false if the event hasn't been handled by the app, true if it has been handled */
        virtual bool OnTouchEvent(TouchEvents event) {
          return false;
        }
        virtual bool OnTouchEvent(uint16_t x, uint16_t y) {
          return false;
        }

      protected:
        DisplayApp* app;
        bool running = true;
      };
    }
  }
}
