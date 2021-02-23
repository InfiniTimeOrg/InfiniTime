//
// Created by florian on 05.04.21.
//

#pragma once

#include <array>
#include "Screen.h"
#include "../LittleVgl.h"
#include <string>


namespace Pinetime::Applications::Screens {
  
  enum class NodeType {
    Num, Add, Sub, Div, Mul, Brc
  };

  class Calculator : public Screen {
  public:
    ~Calculator() override;
    Calculator(DisplayApp* app);
    void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
    bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;
  private:
    //lv_style_t resultStyle;
    bool running;
    lv_obj_t *result, *resultContainer, *returnButton, *buttonMatrix;
  
    char text[31];
    uint8_t position = 0;
  
    void eval();
  
    int getPrecedence(char op);
  
    bool leftAssociative(char op);
  };
  
}


