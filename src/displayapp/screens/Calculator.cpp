#include "Calculator.h"
#include <cmath>


using namespace Pinetime::Applications::Screens;


static void eventHandler(lv_obj_t* obj, lv_event_t event) {
  auto app = static_cast<Calculator*>(obj->user_data);
  app->OnButtonEvent(obj, event);
}

Calculator::~Calculator() {
  lv_obj_clean(lv_scr_act());
}

static const char* buttonMap[] = {
  "7", "8", "9", "<", "\n",
  "4", "5", "6", "+-", "\n",
  "1", "2", "3", "*/", "\n",
  ".", "0", "=", "^", ""
};

Calculator::Calculator(DisplayApp* app) : Screen(app) {
  resultLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(resultLabel, LV_LABEL_LONG_CROP);
  lv_label_set_align(resultLabel, LV_LABEL_ALIGN_RIGHT);
  lv_label_set_text(resultLabel, "0");
  lv_obj_set_size(resultLabel, 190, 20);
  lv_obj_set_pos(resultLabel, 10, 10);

  valueLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_CROP);
  lv_label_set_align(valueLabel, LV_LABEL_ALIGN_RIGHT);
  lv_label_set_text(valueLabel, "0");
  lv_obj_set_size(valueLabel, 190, 20);
  lv_obj_set_pos(valueLabel, 10, 40);

  operationLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(operationLabel, "");
  lv_obj_set_size(operationLabel, 20, 20);
  lv_obj_set_pos(operationLabel, 210, 40);

  buttonMatrix = lv_btnmatrix_create(lv_scr_act(), nullptr);
  buttonMatrix->user_data = this;
  lv_obj_set_event_cb(buttonMatrix, eventHandler);
  lv_btnmatrix_set_map(buttonMatrix, buttonMap);
  lv_obj_set_size(buttonMatrix, 240, 180);
  lv_obj_align(buttonMatrix, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
}

void Calculator::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == buttonMatrix) {
      const char* buttonText = lv_btnmatrix_get_active_btn_text(obj);

      // we only compare the first char because it is enough
      switch (*buttonText) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          // *buttonText is the first char in buttonText
          // "- '0'" results in the int value of the char
          // *10 shifts the value one digit to the left so we can add the new digit
          value = (value * 10) + (*buttonText - '0');
          break;

        case '.':
          break;

        // for every operator we:
        // - eval the current operator if value > 0
        // - then set the new operator
        // - + and - as well as * and / cycle on the same button
        case '+':
          if (value != 0) {
            Eval();
          }
          if (operation == '+') {
            operation = '-';
          } else {
            operation = '+';
          }
          break;
        case '*':
          if (value != 0) {
            Eval();
          }
          if (operation == '*') {
            operation = '/';
          } else {
            operation = '*';
          }
          break;
        case '^':
          if (value != 0) {
            Eval();
          }
          operation = '^';
          break;

        case '<':
          if (value > 0) {
            value = 0;
          } else {
            result = 0;
          }
          operation = ' ';
          break;
        case '=':
          Eval();
          operation = ' ';
          break;
      }
    lv_label_set_text_fmt(valueLabel, "%d", value);
    lv_label_set_text_fmt(resultLabel, "%d", result);
    lv_label_set_text(operationLabel, &operation);
    }
  }
}

void Calculator::Eval() {
  switch (operation) {
    case ' ':
    case '=':
      result = value;
      value = 0;
      break;
    case '+':
      result += value;
      value = 0;
      break;
    case '-':
      result -= value;
      value = 0;
      break;
    case '*':
      result *= value;
      value = 0;
      break;
    case '/':
      if (value != 0) {
        result /= value;
        value = 0;
      }
      break;
    case '^':
      result = pow(result, value);
      value = 0;
      break;
  }
  operation = ' ';
}
