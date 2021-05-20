#include "Calculator.h"
#include <string>
#include <stack>
#include <deque>
#include <math.h>
#include <cmath>
#include <map>

using namespace Pinetime::Applications::Screens;


//Anonymous Namespace for all the structs
namespace {
  struct CalcTreeNode {
    virtual double calculate() = 0;
  };
  
  struct NumNode : CalcTreeNode {
    double value;
    
    double calculate() override {
      return value;
    };
  };
  
  
  struct BinOp : CalcTreeNode {
    CalcTreeNode* left;
    CalcTreeNode* right;
    
    char op;
    
    double calculate() override {
      switch (op) {
        case '^':
          return pow(left->calculate(), right->calculate());
        case 'x':
          return left->calculate() * right->calculate();
        case '/':
          return left->calculate() / right->calculate();
        case '+':
          return left->calculate() + right->calculate();
        case '-':
          return left->calculate() - right->calculate();
      }
      return 0;
    };
    
  };
  
}


static void eventHandler(lv_obj_t* obj, lv_event_t event) {
  auto calc = static_cast<Calculator*>(obj->user_data);
  calc->OnButtonEvent(obj, event);
}

Calculator::~Calculator() {
  lv_obj_clean(lv_scr_act());
}

static const char* buttonMap[] = {"7", "8", "9", "/", "\n",
                                  "4", "5", "6", "x", "\n",
                                  "1", "2", "3", "-", "\n",
                                  ".", "0", "=", "+", "",};

Calculator::Calculator(DisplayApp* app) : Screen(app) {
  result = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(result, LV_LABEL_LONG_BREAK);
  lv_label_set_text(result, "0");
  lv_obj_set_size(result, 180, 60);
  lv_obj_set_pos(result, 0, 0);
  
  returnButton = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(returnButton, 60, 60);
  lv_obj_set_pos(returnButton, 180, 0);
  lv_obj_t* returnLabel;
  returnLabel = lv_label_create(returnButton, nullptr);
  lv_label_set_text(returnLabel, "r");
  lv_obj_align(returnLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
  returnButton->user_data = this;
  lv_obj_set_event_cb(returnButton, eventHandler);
  
  buttonMatrix = lv_btnmatrix_create(lv_scr_act(), nullptr);
  lv_btnmatrix_set_map(buttonMatrix, buttonMap);
  lv_obj_set_size(buttonMatrix, 240, 180);
  lv_obj_set_pos(buttonMatrix, 0, 60);
  buttonMatrix->user_data = this;
  lv_obj_set_event_cb(buttonMatrix, eventHandler);
  running = true;
}

void Calculator::eval() {
  std::stack<char> input(std::deque<char>(0, ' '));
  for (int8_t i = position - 1; i >= 0; i--) {
    input.push(text[i]);
  }
  std::stack<CalcTreeNode*> output(std::deque<CalcTreeNode*>(0, nullptr));
  std::stack<char> operators(std::deque<char>(0, ' '));
  bool expectingNumber = true;
  int8_t sign = +1;
  while (!input.empty()) {
    
    if (isdigit(input.top())) {
      std::string numberStr = "";
      
      while (!input.empty() && (isdigit(input.top()) || input.top() == '.')) {
        numberStr.push_back(input.top());
        input.pop();
      }
      NumNode* number = new NumNode();
      number->value = std::strtod(numberStr.c_str(), nullptr) * sign;
      output.push(number);
      
      sign = +1;
      expectingNumber = false;
      continue;
    }
    if (expectingNumber && input.top() == '+') {
      input.pop();
      continue;
    }
    if (expectingNumber && input.top() == '-') {
      sign *= -1;
      input.pop();
      continue;
    }
    char next = input.top();
    input.pop();
    
    switch (next) {
      case '+':
      case '-':
      case '/':
      case 'x':
      case '^':
        //while ((there is an operator at the top of the operator stack)
        while (!operators.empty()
               //and (the operator at the top of the operator stack is not a left parenthesis))
               && operators.top() == '('
               //and ((the operator at the top of the operator stack has greater precedence)
               && (getPrecedence(operators.top()) > getPrecedence(next)
                   //or (the operator at the top of the operator stack has equal precedence and the token is left associative))
                   || (getPrecedence(operators.top()) == getPrecedence(next) && leftAssociative(next))
               )) {
          auto* node = new BinOp();
          node->right = output.top();
          output.pop();
          node->left = output.top();
          output.pop();
          node->op = next;
          operators.pop();
          output.push(node);
          
        }
        operators.push(next);
        expectingNumber = true;
        break;
      case '(':
        operators.push(next);
        expectingNumber = true;
        break;
      case ')':
        while (operators.top() != '(') {
          auto* node = new BinOp();
          node->right = output.top();
          output.pop();
          node->left = output.top();
          output.pop();
          node->op = operators.top();
          operators.pop();
          output.push(node);
          if (operators.empty()) {
            text[0] = 'm';
            text[1] = 'p';
            position = 2;
            return;
          }
        }
        //discard the left parentheses
        operators.pop();
    }
  }
  while (!operators.empty()) {
    char op = operators.top();
    if (op == ')' || op == '(') {
      text[0] = 'm';
      text[1] = 'p';
      position = 2;
      return;
    }
    auto* node = new BinOp();
    node->right = output.top();
    output.pop();
    node->left = output.top();
    output.pop();
    node->op = op;
    operators.pop();
    output.push(node);
    
  }
  
  //weird workaround because sprintf crashes when trying to use a float
  double resultFloat = output.top()->calculate();
  int32_t upper = resultFloat;
  int32_t lower = std::abs(resultFloat - upper) * 10000;
  if (lower != 0) {
    position = sprintf(text, "%d.%d", upper, lower);
  } else {
    position = sprintf(text, "%d", upper);
  }
  
}

uint8_t Calculator::getPrecedence(char op) {
  switch (op) {
    case '^':
      return 4;
    case 'x':
    case '/':
      return 3;
    case '+':
    case '-':
      return 2;
  }
  running = false;
  return 0;
}

bool Calculator::leftAssociative(char op) {
  switch (op) {
    case '^':
      return false;
    case 'x':
    case '/':
    case '+':
    case '-':
      return true;
  }
  running = false;
  return false;
}

void Calculator::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == buttonMatrix) {
      const char* buttonstr = lv_btnmatrix_get_active_btn_text(obj);
      if (*buttonstr == '=') {
        eval();
      } else {
        
        
        text[position] = *buttonstr;
        position++;
        
      }
    } else if (obj == returnButton) {
      if (position != 0) {
        
        position--;
      } else {
        
        lv_label_set_text(result, "0");
      }
    }
    
    text[position] = '\0';
    lv_label_set_text(result, text);
  }
  
}

bool Calculator::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if (event == Pinetime::Applications::TouchEvents::LongTap) {
    position = 0;
    lv_label_set_text(result, "0");
    return true;
  }
  return false;
}

bool Calculator::Refresh() {
  return running;
}
