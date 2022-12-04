#include "Calculator.h"
#include <cmath>
#include <libraries/log/nrf_log.h>


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
  lv_label_set_text_fmt(resultLabel, "%d", result);
  lv_obj_set_size(resultLabel, 190, 20);
  lv_obj_set_pos(resultLabel, 10, 10);

  valueLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_CROP);
  lv_label_set_align(valueLabel, LV_LABEL_ALIGN_RIGHT);
  lv_label_set_text_fmt(valueLabel, "%d", value);
  lv_obj_set_size(valueLabel, 190, 20);
  lv_obj_set_pos(valueLabel, 10, 40);

  operationLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(operationLabel, &operation);
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
          // if this is true, we already pressed the . button
          if (offset < FIXED_POINT_OFFSET) {
            value += offset * (*buttonText - '0');
            offset /= 10;
          } else {
            value *= 10;
            value += offset * (*buttonText - '0');
          }
          // *buttonText is the first char in buttonText
          // "- '0'" results in the int value of the char
          NRF_LOG_INFO(". offset: %d", offset);
          NRF_LOG_INFO(". value: %d", value);
          NRF_LOG_INFO(". result: %d", result);
          break;

        case '.':
          if (offset == FIXED_POINT_OFFSET) {
            offset /= 10;
          }
          NRF_LOG_INFO(". offset: %d", offset);
          NRF_LOG_INFO(". value: %d", value);
          NRF_LOG_INFO(". result: %d", result);
          break;

        // for every operator we:
        // - eval the current operator if value > FIXED_POINT_OFFSET
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
          if (value != 0) {
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
    
    // show values, spare . if no remainder
    int valueRemainder = value % FIXED_POINT_OFFSET;
    if (valueRemainder || offset < FIXED_POINT_OFFSET) {
      lv_label_set_text_fmt(valueLabel, "%d.%d", value / FIXED_POINT_OFFSET, valueRemainder);
    } else {
      lv_label_set_text_fmt(valueLabel, "%d", value / FIXED_POINT_OFFSET);
    }
    int resultRemainder = result % FIXED_POINT_OFFSET;
    if (resultRemainder) {
      lv_label_set_text_fmt(resultLabel, "%d.%d", result / FIXED_POINT_OFFSET, resultRemainder);
    } else {
      lv_label_set_text_fmt(resultLabel, "%d", result / FIXED_POINT_OFFSET);
    }

    // show operation
    lv_label_set_text_static(operationLabel, &operation);
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
      // fixed point offset was multiplied too
      result /= FIXED_POINT_OFFSET;
      value = 0;
      break;
    case '/':
      if (value != FIXED_POINT_OFFSET) {
        // fixed point offset will be divided too
        result *= FIXED_POINT_OFFSET;
        result /= value;
        value = 0;
      }
      break;

    // we use floats here because pow with fixed point numbers is weird
    case '^':
      float tmp_value = (float)value;
      tmp_value /= FIXED_POINT_OFFSET;

      float tmp_result = (float)result;
      tmp_result /= FIXED_POINT_OFFSET;

      tmp_result = pow(tmp_result, tmp_value);
      result = (long) (tmp_result * FIXED_POINT_OFFSET);

      value = 0;
      break;
  }
  operation = ' ';
  offset = FIXED_POINT_OFFSET;
}
