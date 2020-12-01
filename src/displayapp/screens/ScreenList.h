#pragma once

#include <array>
#include <functional>
#include <memory>
#include "Screen.h"
#include "../DisplayApp.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      template <size_t N>
      class ScreenList : public Screen {
        public:
          ScreenList(DisplayApp* app, std::array<std::function<std::unique_ptr<Screen>()>, N>&& screens)
          : Screen(app), screens{std::move(screens)}, current{this->screens[0]()} {

          }

          ~ScreenList() override {

          }

          bool Refresh() override {
            running = current->Refresh();
            return running;
          }

          bool OnButtonPushed() override {
            running = false;
            return true;
          }

          bool OnTouchEvent(TouchEvents event) override {
            switch (event) {
              case TouchEvents::SwipeDown:
                if (screenIndex > 0) {
                  current.reset(nullptr);
                  app->SetFullRefresh(DisplayApp::FullRefreshDirections::Down);
                  screenIndex--;
                  current = screens[screenIndex]();
                }
                return true;
              case TouchEvents::SwipeUp:
                if (screenIndex < screens.size() - 1) {
                  current.reset(nullptr);
                  app->SetFullRefresh(DisplayApp::FullRefreshDirections::Up);
                  screenIndex++;
                  current = screens[screenIndex]();
                }
                return true;
              default:
                return false;
            }
            return false;
          }

        private:
          bool running = true;
          uint8_t screenIndex = 0;
          std::array<std::function<std::unique_ptr<Screen>()>, N> screens;
          std::unique_ptr<Screen> current;
      };
    }
  }
}