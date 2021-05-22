
#pragma once

#include <array>
#include "Screen.h"
#include "../LittleVgl.h"
#include <string>
#include "components/motor/MotorController.h"


namespace Pinetime {
  namespace Applications {
    namespace Screens {
      
      class Calculator : public Screen {
      public:
        ~Calculator() override;
        
        Calculator(DisplayApp* app, Controllers::MotorController& motorController);
        
        bool Refresh() override;
        
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
      
      private:
        bool running;
        lv_obj_t* result, * resultContainer, * returnButton, * buttonMatrix;
        
        char text[31];
        uint8_t position = 0;
        
        void eval();
        
        Controllers::MotorController& motorController;
        
      };
      
    }
  }
}
