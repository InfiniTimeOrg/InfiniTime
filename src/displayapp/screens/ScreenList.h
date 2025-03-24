#pragma once

#include <array>
#include <functional>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/DisplayApp.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      enum class ScreenListModes { UpDown, RightLeft, LongPress };

      template <size_t N>
      class ScreenList : public Screen {
      public:
        ScreenList(DisplayApp* app,
                   uint8_t initScreen,
                   const std::array<std::function<std::unique_ptr<Screen>()>, N>&& screens,
                   ScreenListModes mode)
          : app {app},
            initScreen {initScreen},
            screens {std::move(screens)},
            mode {mode},
            screenIndex {initScreen},
            current {this->screens[initScreen]()} {
        }

        ScreenList(const ScreenList&) = delete;
        ScreenList& operator=(const ScreenList&) = delete;
        ScreenList(ScreenList&&) = delete;
        ScreenList& operator=(ScreenList&&) = delete;

        ~ScreenList() override {
          lv_obj_clean(lv_scr_act());
        }

        bool OnTouchEvent(TouchEvents event) override {

          if (mode == ScreenListModes::UpDown) {
            switch (event) {
              case TouchEvents::SwipeDown:
                if (screenIndex > 0) {
                  current.reset(nullptr);
                  app->SetFullRefresh(DisplayApp::FullRefreshDirections::Down);
                  screenIndex--;
                  current = screens[screenIndex]();
                  return true;
                } else {
                  return false;
                }

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
          } else if (mode == ScreenListModes::RightLeft) {
            switch (event) {
              case TouchEvents::SwipeRight:
                if (screenIndex > 0) {
                  current.reset(nullptr);
                  app->SetFullRefresh(DisplayApp::FullRefreshDirections::None);
                  screenIndex--;
                  current = screens[screenIndex]();
                  return true;
                } else {
                  return false;
                }

              case TouchEvents::SwipeLeft:
                if (screenIndex < screens.size() - 1) {
                  current.reset(nullptr);
                  app->SetFullRefresh(DisplayApp::FullRefreshDirections::None);
                  screenIndex++;
                  current = screens[screenIndex]();
                }
                return true;
              default:
                return false;
            }
          } else if (event == TouchEvents::LongTap) {
            if (screenIndex < screens.size() - 1) {
              screenIndex++;
            } else {
              screenIndex = 0;
            }
            current.reset(nullptr);
            app->SetFullRefresh(DisplayApp::FullRefreshDirections::None);
            current = screens[screenIndex]();
            return true;
          }

          return false;
        }

      private:
        DisplayApp* app;
        uint8_t initScreen = 0;
        const std::array<std::function<std::unique_ptr<Screen>()>, N> screens;
        ScreenListModes mode = ScreenListModes::UpDown;

        uint8_t screenIndex = 0;
        std::unique_ptr<Screen> current;
      };
    }
  }
}
