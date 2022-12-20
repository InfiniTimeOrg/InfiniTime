#include "Calculator.h"
#include <cmath>
#include <cinttypes>
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Applications::Screens;

static void eventHandler(lv_obj_t* obj, lv_event_t event) {
  auto app = static_cast<Calculator*>(obj->user_data);
  app->OnButtonEvent(obj, event);
}

Calculator::~Calculator() {
  lv_obj_clean(lv_scr_act());
}

static const char* buttonMap[] = {"7", "8", "9", "<", "\n", "4", "5", "6", "+-", "\n", "1", "2", "3", "*/", "\n", ".", "0", "=", "^", ""};

Calculator::Calculator(DisplayApp* app) : Screen(app) {
  resultLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(resultLabel, LV_LABEL_LONG_CROP);
  lv_label_set_align(resultLabel, LV_LABEL_ALIGN_RIGHT);
  lv_label_set_text_fmt(resultLabel, "%" PRId64, result);
  lv_obj_set_size(resultLabel, 145, 20);
  lv_obj_set_pos(resultLabel, 10, 5);

  valueLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_CROP);
  lv_label_set_align(valueLabel, LV_LABEL_ALIGN_RIGHT);
  lv_label_set_text_fmt(valueLabel, "%" PRId64, value);
  lv_obj_set_size(valueLabel, 145, 20);
  lv_obj_set_pos(valueLabel, 10, 35);

  operationLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(operationLabel, operation);
  lv_obj_set_size(operationLabel, 20, 20);
  lv_obj_set_pos(operationLabel, 203, 35);

  buttonMatrix = lv_btnmatrix_create(lv_scr_act(), nullptr);
  buttonMatrix->user_data = this;
  lv_obj_set_event_cb(buttonMatrix, eventHandler);
  lv_btnmatrix_set_map(buttonMatrix, buttonMap);
  lv_obj_set_size(buttonMatrix, 238, 180);
  lv_obj_set_style_local_pad_inner(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_pad_top(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_pad_bottom(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_pad_left(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_pad_right(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_align(buttonMatrix, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
}

void Calculator::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == buttonMatrix) {
      const char* buttonText = lv_btnmatrix_get_active_btn_text(obj);

      if (buttonText == nullptr) {
        return;
      }

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
            // *buttonText is the first char in buttonText
            // "- '0'" results in the int value of the char
            value += offset * (*buttonText - '0');
            offset /= 10;
          } else if (value <= MAX_VALUE / 10) {
            value *= 10;
            value += offset * (*buttonText - '0');
          }
          UpdateValueLabel();

          NRF_LOG_INFO(". offset: %" PRId64, offset);
          NRF_LOG_INFO(". value: %" PRId64, value);
          NRF_LOG_INFO(". result: %" PRId64, result);
          break;

        case '.':
          if (offset == FIXED_POINT_OFFSET) {
            offset /= 10;
          }
          UpdateValueLabel();

          NRF_LOG_INFO(". offset: %" PRId64, offset);
          NRF_LOG_INFO(". value: %" PRId64, value);
          NRF_LOG_INFO(". result: %" PRId64, result);
          break;

        // for every operator we:
        // - eval the current operator if value > FIXED_POINT_OFFSET
        // - then set the new operator
        // - + and - as well as * and / cycle on the same button
        case '+':
          if (value != 0) {
            Eval();
          }
          if (*operation == '+') {
            *operation = '-';
          } else {
            *operation = '+';
          }
          lv_label_refr_text(operationLabel);
          break;
        case '*':
          if (value != 0) {
            Eval();
          }
          if (*operation == '*') {
            *operation = '/';
          } else {
            *operation = '*';
          }
          lv_label_refr_text(operationLabel);
          break;
        case '^':
          if (value != 0) {
            Eval();
          }
          *operation = '^';
          lv_label_refr_text(operationLabel);
          break;

        case '<':
          offset = FIXED_POINT_OFFSET;
          if (value != 0) {
            value = 0;
          } else {
            result = 0;
            UpdateResultLabel();
          }
          UpdateValueLabel();

          *operation = ' ';
          lv_label_refr_text(operationLabel);
          break;
        case '=':
          Eval();
          *operation = ' ';
          lv_label_refr_text(operationLabel);
          break;
      }
    }
  }
}

void Calculator::UpdateResultLabel() {
  int64_t integer = result / FIXED_POINT_OFFSET;
  int64_t remainder = result % FIXED_POINT_OFFSET;

  if (remainder == 0) {
    lv_label_set_text_fmt(resultLabel, "%" PRId64, integer);
    return;
  }

  if (remainder < 0) {
    remainder = -remainder;
  }

  uint8_t min_width = N_DECIMALS;

  // cut "0"-digits on the right
  while ((remainder > 0) && (remainder % 10 == 0)) {
    remainder /= 10;
    min_width--;
  }

  lv_label_set_text_fmt(resultLabel, "%" PRId64 ".%0*" PRId64, integer, min_width, remainder);
}

void Calculator::UpdateValueLabel() {
  int64_t integer = value / FIXED_POINT_OFFSET;
  int64_t remainder = value % FIXED_POINT_OFFSET;

  int64_t printRemainder = remainder < 0 ? -remainder : remainder;

  uint8_t min_width = 0;
  int64_t tmp_offset = offset;

  // TODO there has to be a simpler way to do this
  if (tmp_offset == 0) {
    tmp_offset = 1;
    min_width = 1;
  }
  while (tmp_offset < FIXED_POINT_OFFSET) {
    tmp_offset *= 10;
    min_width++;
  }
  min_width--;

  for (uint8_t i = min_width; i < N_DECIMALS; i++) {
    printRemainder /= 10;
  }

  if (offset == FIXED_POINT_OFFSET) {
    lv_label_set_text_fmt(valueLabel, "%" PRId64, integer);
  } else if ((offset == (FIXED_POINT_OFFSET / 10)) && (remainder == 0)) {
    lv_label_set_text_fmt(valueLabel, "%" PRId64 ".", integer);
  } else {
    lv_label_set_text_fmt(valueLabel, "%" PRId64 ".%0*" PRId64, integer, min_width, printRemainder);
  }
}

void Calculator::Eval() {
  switch (*operation) {
    case ' ':
    case '=':
      result = value;
      value = 0;
      break;
    case '+':
      // check for overflow
      if (((result > 0) && (value > (MAX_VALUE - result))) ||
          ((result < 0) && (value < (MIN_VALUE - result)))) {
        break;
      }

      result += value;
      value = 0;
      break;
    case '-':
      // check for overflow
      if (((result < 0) && (value > (MAX_VALUE + result))) ||
          ((result > 0) && (value < (MIN_VALUE + result)))) {
        break;
      }

      result -= value;
      value = 0;
      break;
    case '*':
      // check for overflow
      // while dividing we eliminate the fixed point offset
      // therefore we have to multiply it again for the comparison with value
      if (((result != 0) && (value > (FIXED_POINT_OFFSET * (MAX_VALUE / result)))) ||
          ((result != 0) && (value < (FIXED_POINT_OFFSET * (MIN_VALUE / result))))) {
        break;
      }

      result *= value;
      // fixed point offset was multiplied too
      result /= FIXED_POINT_OFFSET;
      value = 0;
      break;
    case '/':
      if (value != 0) {
        // fixed point offset will be divided too
        result *= FIXED_POINT_OFFSET;
        result /= value;
        value = 0;
      }
      break;

    // we use floats here because pow with fixed point numbers is weird
    case '^':
      double tmp_value = static_cast<double>(value);
      tmp_value /= static_cast<double>(FIXED_POINT_OFFSET);

      double tmp_result = static_cast<double>(result);
      tmp_result /= static_cast<double>(FIXED_POINT_OFFSET);

      tmp_result = pow(tmp_result, tmp_value);
      tmp_result *= static_cast<double>(FIXED_POINT_OFFSET);
      result = static_cast<int64_t>(tmp_result);

      value = 0;
      break;
  }
  *operation = ' ';
  offset = FIXED_POINT_OFFSET;

  UpdateValueLabel();
  UpdateResultLabel();
}
