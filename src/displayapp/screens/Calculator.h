
#pragma once

#include <array>
#include "Screen.h"
#include "../LittleVgl.h"
#include <string>


namespace Pinetime {
  namespace Applications {
    namespace Screens {
      
      class Calculator : public Screen {
      public:
        ~Calculator() override;
        
        Calculator(DisplayApp* app);
  
        bool Refresh() override;
        
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
      
      private:
        bool running;
        lv_obj_t* result, * resultContainer, * returnButton, * buttonMatrix;
        
        char text[31];
        uint8_t position = 0;
        
        void eval();
        
        uint8_t getPrecedence(char op);
        
        bool leftAssociative(char op);
      };
      
    }
  }
}
