#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "displayapp/screens/List.h"
#include "Timer.h"


namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class TimerList : public List {
        enum Mode {Normal, Set};
      public:
        
        explicit TimerList(uint8_t screenID,
                              uint8_t numScreens,
                              DisplayApp* app,
                              Controllers::TimerController& timerController);
        
        ~TimerList() override;
        
        bool Refresh() override;
        
        void OnButtonEvent(lv_obj_t* object, lv_event_t event);
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event);
      
      private:
        Mode currentMode = Normal;
        Controllers::TimerController& timerController;
        
        
      };
    }
  }
}
