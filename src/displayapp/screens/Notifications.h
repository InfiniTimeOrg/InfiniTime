#pragma once

#include <functional>
#include <vector>

#include "Screen.h"
#include "ScreenList.h"


namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Notifications : public Screen {
        public:
          explicit Notifications(DisplayApp* app);
          ~Notifications() override;

          bool Refresh() override;
          bool OnButtonPushed() override;
          bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

        private:
          ScreenList<3> screens;
          bool running = true;
          std::unique_ptr<Screen> CreateScreen1();
          std::unique_ptr<Screen> CreateScreen2();
          std::unique_ptr<Screen> CreateScreen3();

          class NotificationItem : public Screen {
            public:
              NotificationItem(DisplayApp* app, const char* title, const char* msg, uint8_t notifNr, uint8_t notifNb);
              NotificationItem(DisplayApp* app, const char* title1, const char* msg1, const char* title2, const char* msg2, uint8_t notifNr, uint8_t notifNb);
              ~NotificationItem() override;
              bool Refresh() override {return false;}

            private:
              uint8_t notifNr = 0;
              uint8_t notifNb = 0;
              char pageText[4];
          };
      };
    }
  }
}
