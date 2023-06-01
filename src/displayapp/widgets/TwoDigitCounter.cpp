#include "displayapp/widgets/TwoDigitCounter.h"
#include "displayapp/InfiniTimeTheme.h"
#include <algorithm>

using namespace Pinetime::Applications::Widgets;

namespace {

    void tensUpBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<TwoDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED) {
            widget->UpBtnPressed(true);
        } else if (event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->resetTotal();
        }
    }

    void tensDownBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<TwoDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->DownBtnPressed(true);
        }
    }

    void upBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<TwoDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->UpBtnPressed(false);
        }
    }

    void downBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<TwoDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->DownBtnPressed(false);
        }
    }
}

TwoDigitCounter::TwoDigitCounter(int min, int max, lv_font_t& font) 
: min {min}, max {max}, font {font} {
}

void TwoDigitCounter::resetTotal() {
    value = 40;
    UpdateLabel();
}

void TwoDigitCounter::UpBtnPressed(bool isTens) {
    if (isTens) {
        value = value + 10;
    } else {
        value++;
    }
    UpdateLabel();

    if (ValueChangedHandler != nullptr) {
        ValueChangedHandler(userData);
    }
};

void TwoDigitCounter::DownBtnPressed(bool isTens) {
    if (isTens) {
        value = value - 10;
    } else {
        value--;
    }
    if (value <= 0) {
        value = 0;
    }
    UpdateLabel();

    if (ValueChangedHandler != nullptr) {
        ValueChangedHandler(userData);
    }
};

void TwoDigitCounter::UpdateLabel() {
    if ( value <= 0 ) {
        lv_label_set_text(number, "#FF1414 0#");
    } else if ( value < 5 ) {
        lv_label_set_text_fmt(number, "#FF1414  %.*i#", 0, value);
    } else if ( value < 12 ) {
        lv_label_set_text_fmt(number, "#FF1694  %.*i#", 0, value);
    }  else if ( value < 25 ) {
        lv_label_set_text_fmt(number, "#FDCE2A  %.*i#", 0, value);
    } 
    else {
        lv_label_set_text_fmt(number, "#00FF7F %.*i#", 0, value);
    }
}

void TwoDigitCounter::SetValueChangedEventCallback(void* userData, void (*handler)(void* userData)) {
  this->userData = userData;
  this->ValueChangedHandler = handler;
}

void TwoDigitCounter::Create() {
  counterContainer = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(counterContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);

  number = lv_label_create(counterContainer, nullptr);
  lv_obj_set_style_local_text_font(number, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &font);
  lv_obj_align(number, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_recolor(number, true);
  lv_obj_set_auto_realign(number, true);

  static constexpr uint8_t padding = 5;
  const uint8_t width = std::max(lv_obj_get_width(number) + padding * 2, 58);
  static constexpr uint8_t btnHeight = 50;
  const uint8_t containerHeight = btnHeight * 2 + lv_obj_get_height(number) + padding * 2;

  lv_obj_set_size(counterContainer, width * 3, containerHeight);

  UpdateLabel();

  tensUpBtn = lv_btn_create(counterContainer, nullptr);
  lv_obj_set_style_local_bg_color(tensUpBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_size(tensUpBtn, width, btnHeight);
  lv_obj_align(tensUpBtn, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  tensUpBtn->user_data = this;
  lv_obj_set_event_cb(tensUpBtn, tensUpBtnEventHandler);

  lv_obj_t* tensUpLabel = lv_label_create(tensUpBtn, nullptr);
  lv_obj_set_style_local_text_font(tensUpLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(tensUpLabel, "+");
  lv_obj_align(tensUpLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

  upBtn = lv_btn_create(counterContainer, nullptr);
  lv_obj_set_style_local_bg_color(upBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_size(upBtn, width, btnHeight);
  lv_obj_align(upBtn, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  upBtn->user_data = this;
  lv_obj_set_event_cb(upBtn, upBtnEventHandler);

  lv_obj_t* upLabel = lv_label_create(upBtn, nullptr);
  lv_obj_set_style_local_text_font(upLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(upLabel, "+");
  lv_obj_align(upLabel, nullptr, LV_ALIGN_CENTER, 0, 0);


  tensDownBtn = lv_btn_create(counterContainer, nullptr);
  lv_obj_set_style_local_bg_color(tensDownBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_size(tensDownBtn, width, btnHeight);
  lv_obj_align(tensDownBtn, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  tensDownBtn->user_data = this;
  lv_obj_set_event_cb(tensDownBtn, tensDownBtnEventHandler);

  lv_obj_t* tensDownLabel = lv_label_create(tensDownBtn, nullptr);
  lv_obj_set_style_local_text_font(tensDownLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_static(tensDownLabel, "-");
  lv_obj_align(tensDownLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

  downBtn = lv_btn_create(counterContainer, nullptr);
  lv_obj_set_style_local_bg_color(downBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_size(downBtn, width, btnHeight);
  lv_obj_align(downBtn, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
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

  tensUpperLine = LineCreate();
  lv_obj_align(tensUpperLine, tensUpBtn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  tensLowerLine = LineCreate();
  lv_obj_align(tensLowerLine, tensDownBtn, LV_ALIGN_OUT_TOP_MID, 0, -1);

  upperLine = LineCreate();
  lv_obj_align(upperLine, upBtn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  lowerLine = LineCreate();
  lv_obj_align(lowerLine, downBtn, LV_ALIGN_OUT_TOP_MID, 0, -1);
}