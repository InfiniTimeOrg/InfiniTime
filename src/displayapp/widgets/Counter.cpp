#include "displayapp/widgets/Counter.h"

using namespace Pinetime::Applications::Widgets;

namespace {
  void upBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* widget = static_cast<Counter*>(obj->user_data);
    if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
      widget->Increment();
    }
  }

  void downBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* widget = static_cast<Counter*>(obj->user_data);
    if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
      widget->Decrement();
    }
  }
}

Counter::Counter(int min, int max) : min {min}, max {max} {
}

void Counter::Increment() {
  value++;
  if (value > max) {
    value = min;
  }
  UpdateLabel();
};

void Counter::Decrement() {
  value--;
  if (value < min) {
    value = max;
  }
  UpdateLabel();
};

void Counter::SetValue(int newValue) {
  value = newValue;
  UpdateLabel();
}

void Counter::HideControls() {
  lv_obj_set_hidden(upBtn, true);
  lv_obj_set_hidden(downBtn, true);
  lv_obj_set_hidden(upperLine, true);
  lv_obj_set_hidden(lowerLine, true);
  lv_obj_set_style_local_bg_opa(counterContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
}
void Counter::ShowControls() {
  lv_obj_set_hidden(upBtn, false);
  lv_obj_set_hidden(downBtn, false);
  lv_obj_set_hidden(upperLine, false);
  lv_obj_set_hidden(lowerLine, false);
  lv_obj_set_style_local_bg_opa(counterContainer, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
}

void Counter::UpdateLabel() {
  lv_label_set_text_fmt(number, "%.2i", value);
}

void Counter::Create() {
  constexpr lv_color_t bgColor = LV_COLOR_MAKE(0x38, 0x38, 0x38);

  counterContainer = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(counterContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, bgColor);

  number = lv_label_create(counterContainer, nullptr);
  lv_obj_set_style_local_text_font(number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_align(number, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_auto_realign(number, true);
  lv_label_set_text_static(number, "00");

  static constexpr uint8_t padding = 5;
  const uint8_t width = lv_obj_get_width(number) + padding * 2;
  static constexpr uint8_t btnHeight = 50;
  const uint8_t containerHeight = btnHeight * 2 + lv_obj_get_height(number) + padding * 2;

  lv_obj_set_size(counterContainer, width, containerHeight);

  UpdateLabel();

  upBtn = lv_btn_create(counterContainer, nullptr);
  lv_obj_set_style_local_bg_color(upBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, bgColor);
  lv_obj_set_size(upBtn, width, btnHeight);
  lv_obj_align(upBtn, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);
  upBtn->user_data = this;
  lv_obj_set_event_cb(upBtn, upBtnEventHandler);

  lv_obj_t* upLabel = lv_label_create(upBtn, nullptr);
  lv_obj_set_style_local_text_font(upLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(upLabel, "+");
  lv_obj_align(upLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

  downBtn = lv_btn_create(counterContainer, nullptr);
  lv_obj_set_style_local_bg_color(downBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, bgColor);
  lv_obj_set_size(downBtn, width, btnHeight);
  lv_obj_align(downBtn, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  downBtn->user_data = this;
  lv_obj_set_event_cb(downBtn, downBtnEventHandler);

  lv_obj_t* downLabel = lv_label_create(downBtn, nullptr);
  lv_obj_set_style_local_text_font(downLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(downLabel, "-");
  lv_obj_align(downLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

  linePoints[0] = {0, 0};
  linePoints[1] = {width, 0};

  auto LineCreate = [&]() {
    lv_obj_t* line = lv_line_create(counterContainer, nullptr);
    lv_line_set_points(line, linePoints, 2);
    lv_obj_set_style_local_line_width(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_line_color(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_line_opa(line, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
    return line;
  };

  upperLine = LineCreate();
  lv_obj_align(upperLine, upBtn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  lowerLine = LineCreate();
  lv_obj_align(lowerLine, downBtn, LV_ALIGN_OUT_TOP_MID, 0, -1);
}
