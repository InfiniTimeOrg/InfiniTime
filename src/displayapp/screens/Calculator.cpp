#include <cmath>
#include <cinttypes>
#include <libraries/log/nrf_log.h>
#include "Calculator.h"
#include "displayapp/InfiniTimeTheme.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

static void eventHandler(lv_obj_t* obj, lv_event_t event) {
  auto app = static_cast<Calculator*>(obj->user_data);
  app->OnButtonEvent(obj, event);
}

Calculator::~Calculator() {
  lv_obj_clean(lv_scr_act());
}

static const char* buttonMap[] = {
  "7", "8", "9", Symbols::backspace, "\n", "4", "5", "6", "+ -", "\n", "1", "2", "3", "* /", "\n", "0", ".", "(-)", "=", ""};

Calculator::Calculator() {
  resultLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(resultLabel, LV_LABEL_LONG_CROP);
  lv_label_set_align(resultLabel, LV_LABEL_ALIGN_RIGHT);
  lv_label_set_text_fmt(resultLabel, "%" PRId64, result);
  lv_obj_set_size(resultLabel, 200, 20);
  lv_obj_set_pos(resultLabel, 10, 5);

  valueLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(valueLabel, LV_LABEL_LONG_CROP);
  lv_label_set_align(valueLabel, LV_LABEL_ALIGN_RIGHT);
  lv_label_set_text_fmt(valueLabel, "%" PRId64, value);
  lv_obj_set_size(valueLabel, 200, 20);
  lv_obj_set_pos(valueLabel, 10, 35);

  buttonMatrix = lv_btnmatrix_create(lv_scr_act(), nullptr);
  buttonMatrix->user_data = this;
  lv_obj_set_event_cb(buttonMatrix, eventHandler);
  lv_btnmatrix_set_map(buttonMatrix, buttonMap);
  lv_btnmatrix_set_one_check(buttonMatrix, true);
  lv_obj_set_size(buttonMatrix, 238, 180);
  lv_obj_set_style_local_bg_color(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_pad_inner(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_pad_top(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_pad_bottom(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_pad_left(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_pad_right(buttonMatrix, LV_BTNMATRIX_PART_BG, LV_STATE_DEFAULT, 1);
  lv_obj_align(buttonMatrix, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  lv_obj_set_style_local_bg_opa(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_OPA_COVER);
  lv_obj_set_style_local_bg_grad_stop(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, 128);
  lv_obj_set_style_local_bg_main_stop(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, 128);
}

void Calculator::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if ((obj == buttonMatrix) && (event == LV_EVENT_PRESSED)) {
    HandleInput();
  }
}

void Calculator::HandleInput() {
  const char* buttonText = lv_btnmatrix_get_active_btn_text(buttonMatrix);

  if (buttonText == nullptr) {
    return;
  }

  if ((equalSignPressed && (*buttonText != '=')) || (error != Error::None)) {
    ResetInput();
    UpdateOperation();
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
    case '9': {
      // *buttonText is the first char in buttonText
      // "- '0'" results in the int value of the char
      auto digit = (*buttonText) - '0';
      auto sign = (value < 0) ? -1 : 1;

      // if this is true, we already pressed the . button
      if (offset < FIXED_POINT_OFFSET) {
        value += sign * offset * digit;
        offset /= 10;
      } else if (value <= MAX_VALUE / 10) {
        value *= 10;
        value += sign * offset * digit;
      }

      NRF_LOG_INFO(". offset: %" PRId64, offset);
      NRF_LOG_INFO(". value: %" PRId64, value);
      NRF_LOG_INFO(". result: %" PRId64, result);
    } break;

    // unary minus
    case '(':
      value = -value;

      NRF_LOG_INFO(". offset: %" PRId64, offset);
      NRF_LOG_INFO(". value: %" PRId64, value);
      NRF_LOG_INFO(". result: %" PRId64, result);

      break;

    case '.':
      if (offset == FIXED_POINT_OFFSET) {
        offset /= 10;
      }

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
        ResetInput();
      }

      switch (operation) {
        case '+':
          operation = '-';
          break;
        case '-':
          operation = ' ';
          break;
        default:
          operation = '+';
          break;
      }
      UpdateOperation();
      break;

    case '*':
      if (value != 0) {
        Eval();
        ResetInput();
      }

      switch (operation) {
        case '*':
          operation = '/';
          break;
        case '/':
          operation = ' ';
          break;
        default:
          operation = '*';
          break;
      }
      UpdateOperation();
      break;

    // this is a little hacky because it matches only the first char
    case Symbols::backspace[0]:
      if (value != 0) {
        // delete one value digit
        if (offset < FIXED_POINT_OFFSET) {
          if (offset == 0) {
            offset = 1;
          } else {
            offset *= 10;
          }
        } else {
          value /= 10;
        }
        if (offset < FIXED_POINT_OFFSET) {
          value -= value % (10 * offset);
        } else {
          value -= value % offset;
        }
      } else if (offset < FIXED_POINT_OFFSET) {
        if (offset == 0) {
          offset = 1;
        } else {
          offset *= 10;
        }
      } else {
        // reset the result
        result = 0;
      }

      NRF_LOG_INFO(". offset: %" PRId64, offset);
      NRF_LOG_INFO(". value: %" PRId64, value);
      NRF_LOG_INFO(". result: %" PRId64, result);

      if (value == 0) {
        operation = ' ';
        UpdateOperation();
      }
      break;

    case '=':
      equalSignPressed = true;
      Eval();
      // If the operation is ' ' then we move the value to the result.
      // We reset the input after this.
      // This seems more convenient.
      if (operation == ' ') {
        ResetInput();
      }

      NRF_LOG_INFO(". offset: %" PRId64, offset);
      NRF_LOG_INFO(". value: %" PRId64, value);
      NRF_LOG_INFO(". result: %" PRId64, result);

      break;
  }

  UpdateValueLabel();
  UpdateResultLabel();
}

void Calculator::UpdateOperation() const {
  switch (operation) {
    case '+':
      lv_obj_set_style_local_bg_grad_dir(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_GRAD_DIR_HOR);
      lv_obj_set_style_local_bg_color(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, Colors::deepOrange);
      lv_obj_set_style_local_bg_grad_color(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, Colors::bgAlt);
      lv_btnmatrix_set_btn_ctrl(buttonMatrix, 7, LV_BTNMATRIX_CTRL_CHECK_STATE);
      break;
    case '-':
      lv_obj_set_style_local_bg_grad_dir(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_GRAD_DIR_HOR);
      lv_obj_set_style_local_bg_color(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, Colors::bgAlt);
      lv_obj_set_style_local_bg_grad_color(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, Colors::deepOrange);
      lv_btnmatrix_set_btn_ctrl(buttonMatrix, 7, LV_BTNMATRIX_CTRL_CHECK_STATE);
      break;
    case '*':
      lv_obj_set_style_local_bg_grad_dir(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_GRAD_DIR_HOR);
      lv_obj_set_style_local_bg_color(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, Colors::deepOrange);
      lv_obj_set_style_local_bg_grad_color(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, Colors::bgAlt);
      lv_btnmatrix_set_btn_ctrl(buttonMatrix, 11, LV_BTNMATRIX_CTRL_CHECK_STATE);
      break;
    case '/':
      lv_obj_set_style_local_bg_grad_dir(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, LV_GRAD_DIR_HOR);
      lv_obj_set_style_local_bg_color(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, Colors::bgAlt);
      lv_obj_set_style_local_bg_grad_color(buttonMatrix, LV_BTNMATRIX_PART_BTN, LV_STATE_CHECKED, Colors::deepOrange);
      lv_btnmatrix_set_btn_ctrl(buttonMatrix, 11, LV_BTNMATRIX_CTRL_CHECK_STATE);
      break;
    default:
      lv_btnmatrix_clear_btn_ctrl_all(buttonMatrix, LV_BTNMATRIX_CTRL_CHECK_STATE);
      break;
  }
}

void Calculator::ResetInput() {
  value = 0;
  offset = FIXED_POINT_OFFSET;
  operation = ' ';
  equalSignPressed = false;
  error = Error::None;
}

void Calculator::UpdateResultLabel() const {
  int64_t integer = result / FIXED_POINT_OFFSET;
  int64_t remainder = result % FIXED_POINT_OFFSET;
  bool negative = (remainder < 0);

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

  if ((integer == 0) && negative) {
    lv_label_set_text_fmt(resultLabel, "-0.%0*" PRId64, min_width, remainder);
  } else {
    lv_label_set_text_fmt(resultLabel, "%" PRId64 ".%0*" PRId64, integer, min_width, remainder);
  }
}

void Calculator::UpdateValueLabel() {
  switch (error) {
    case Error::TooLarge:
      lv_label_set_text_static(valueLabel, "too large");
      break;
    case Error::ZeroDivision:
      lv_label_set_text_static(valueLabel, "zero division");
      break;
    case Error::None:
    default: {
      int64_t integer = value / FIXED_POINT_OFFSET;
      int64_t remainder = value % FIXED_POINT_OFFSET;
      bool negative = (remainder < 0);

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

      if ((integer == 0) && negative) {
        lv_label_set_text_fmt(valueLabel, "-0.%0*" PRId64, min_width, printRemainder);
      } else if (offset == FIXED_POINT_OFFSET) {
        lv_label_set_text_fmt(valueLabel, "%" PRId64, integer);
      } else if ((offset == (FIXED_POINT_OFFSET / 10)) && (remainder == 0)) {
        lv_label_set_text_fmt(valueLabel, "%" PRId64 ".", integer);
      } else {
        lv_label_set_text_fmt(valueLabel, "%" PRId64 ".%0*" PRId64, integer, min_width, printRemainder);
      }
    } break;
  }
}

// update the result based on value and operation
void Calculator::Eval() {
  switch (operation) {
    case ' ':
      result = value;
      break;

    case '+':
      // check for overflow
      if (((result > 0) && (value > (MAX_VALUE - result))) || ((result < 0) && (value < (MIN_VALUE - result)))) {
        error = Error::TooLarge;
        break;
      }

      result += value;
      break;
    case '-':
      // check for overflow
      if (((result < 0) && (value > (MAX_VALUE + result))) || ((result > 0) && (value < (MIN_VALUE + result)))) {
        error = Error::TooLarge;
        break;
      }

      result -= value;
      break;
    case '*':
      // check for overflow
      // while dividing we eliminate the fixed point offset
      // therefore we have to multiply it again for the comparison with value
      // we also assume here that MAX_VALUE == -MIN_VALUE
      if ((result != 0) && (std::abs(value) > (FIXED_POINT_OFFSET * (MAX_VALUE / std::abs(result))))) {
        error = Error::TooLarge;
        break;
      }

      result *= value;
      // fixed point offset was multiplied too
      result /= FIXED_POINT_OFFSET;
      break;
    case '/':
      // check for zero division
      if (value == 0) {
        error = Error::ZeroDivision;
        break;
      }

      // fixed point offset will be divided too
      result *= FIXED_POINT_OFFSET;
      result /= value;
      break;

    default:
      break;
  }
}
