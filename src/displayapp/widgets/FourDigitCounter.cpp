#include <algorithm>
#include "displayapp/widgets/FourDigitCounter.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

namespace {
    void thousandsUpBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<FourDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED) {
            widget->UpBtnPressed(Thousands);
        } else if (event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->resetTotal();
        }
    }

    void thousandsDownBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<FourDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->DownBtnPressed(Thousands);
        }
    }

    void hundredsUpBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<FourDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->UpBtnPressed(Hundreds);
        }
    }

    void hundredsDownBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<FourDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->DownBtnPressed(Hundreds);
        }
    }

    void tensUpBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<FourDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->UpBtnPressed(Tens);
        }
    }

    void tensDownBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
        auto* widget = static_cast<FourDigitCounter*>(obj->user_data);
        if (event == LV_EVENT_SHORT_CLICKED || event == LV_EVENT_LONG_PRESSED_REPEAT) {
            widget->DownBtnPressed(Tens);
        }
    }
}

FourDigitCounter::FourDigitCounter(int min, int max, lv_font_t& font)
: min {min}, max {max}, font {font} {
}

void FourDigitCounter::resetTotal() {
    yugiohLife = 8000;
    UpdateLabel();
}

void FourDigitCounter::UpBtnPressed(NumberPlace numberPlace) {
    switch(numberPlace) {
        case Thousands:
            yugiohLife = yugiohLife + 1000;
            break;
        case Hundreds:
            yugiohLife = yugiohLife + 100;
            break;
        case Tens:
            yugiohLife = yugiohLife + 50;
            break;
    }
    UpdateLabel();

    if (ValueChangedHandler != nullptr) {
        ValueChangedHandler(userData);
    }
};

void FourDigitCounter::DownBtnPressed(NumberPlace numberPlace) {
    switch(numberPlace) {
        case Thousands:
            yugiohLife = yugiohLife - 1000;
            break;
        case Hundreds:
            yugiohLife = yugiohLife - 100;
            break;
        case Tens:
            yugiohLife = yugiohLife - 50;
            break;
    }
    UpdateLabel();

    if (ValueChangedHandler != nullptr) {
        ValueChangedHandler(userData);
    }
};

void FourDigitCounter::UpdateLabel() {
    if ( yugiohLife <= 0 ) {
        lv_label_set_text(number, "#FF1414 0000#");
    } else if ( yugiohLife < 1000 ) {
        lv_label_set_text_fmt(number, "#FF1414  %.*i#", 1, yugiohLife);
    } else if ( yugiohLife == 1000) {
        lv_label_set_text_fmt(number, "#FF1414  %.*i#", 0, yugiohLife);
    } else if ( yugiohLife <= 3000 ) {
        lv_label_set_text_fmt(number, "#FF1694  %.*i#", 0, yugiohLife);
    }  else if ( yugiohLife <= 4500 ) {
        lv_label_set_text_fmt(number, "#FDCE2A  %.*i#", 0, yugiohLife);
    } else if ( yugiohLife <= 6000 ) {
        lv_label_set_text_fmt(number, "#4166F5  %.*i#", 0, yugiohLife);
    }
    else {
        lv_label_set_text_fmt(number, "#00FF7F %.*i#", 0, yugiohLife);
    }
}

void FourDigitCounter::SetValueChangedEventCallback(void* userData, void (*handler)(void* userData)) {
  this->userData = userData;
  this->ValueChangedHandler = handler;
}

void FourDigitCounter::Create() {
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

    lv_obj_set_size(counterContainer, width * 4, containerHeight);

    UpdateLabel();

    thousandsUpBtn = lv_btn_create(counterContainer, nullptr);
    lv_obj_set_style_local_bg_color(thousandsUpBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_size(thousandsUpBtn, width, btnHeight);
    lv_obj_align(thousandsUpBtn, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    thousandsUpBtn->user_data = this;
    lv_obj_set_event_cb(thousandsUpBtn, thousandsUpBtnEventHandler);

    lv_obj_t* thousandsUpLabel = lv_label_create(thousandsUpBtn, nullptr);
    lv_obj_set_style_local_text_font(thousandsUpLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
    lv_label_set_text_static(thousandsUpLabel, "+");
    lv_obj_align(thousandsUpLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

    thousandsDownBtn = lv_btn_create(counterContainer, nullptr);
    lv_obj_set_style_local_bg_color(thousandsDownBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_size(thousandsDownBtn, width, btnHeight);
    lv_obj_align(thousandsDownBtn, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    thousandsDownBtn->user_data = this;
    lv_obj_set_event_cb(thousandsDownBtn, thousandsDownBtnEventHandler);

    lv_obj_t* thousandsDownLabel = lv_label_create(thousandsDownBtn, nullptr);
    lv_obj_set_style_local_text_font(thousandsDownLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
    lv_label_set_text_static(thousandsDownLabel, "-");
    lv_obj_align(thousandsDownLabel, nullptr, LV_ALIGN_CENTER, 0, 0);





    hundredsUpBtn = lv_btn_create(counterContainer, nullptr);
    lv_obj_set_style_local_bg_color(hundredsUpBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_size(hundredsUpBtn, width, btnHeight);
    lv_obj_align(hundredsUpBtn, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);
    hundredsUpBtn->user_data = this;
    lv_obj_set_event_cb(hundredsUpBtn, hundredsUpBtnEventHandler);

    lv_obj_t* hundredsUpLabel = lv_label_create(hundredsUpBtn, nullptr);
    lv_obj_set_style_local_text_font(hundredsUpLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
    lv_label_set_text_static(hundredsUpLabel, "+");
    lv_obj_align(hundredsUpLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

    hundredsDownBtn = lv_btn_create(counterContainer, nullptr);
    lv_obj_set_style_local_bg_color(hundredsDownBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_size(hundredsDownBtn, width, btnHeight);
    lv_obj_align(hundredsDownBtn, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    hundredsDownBtn->user_data = this;
    lv_obj_set_event_cb(hundredsDownBtn, hundredsDownBtnEventHandler);

    lv_obj_t* hundredsDownLabel = lv_label_create(hundredsDownBtn, nullptr);
    lv_obj_set_style_local_text_font(hundredsDownLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
    lv_label_set_text_static(hundredsDownLabel, "-");
    lv_obj_align(hundredsDownLabel, nullptr, LV_ALIGN_CENTER, 0, 0);





    tensUpBtn = lv_btn_create(counterContainer, nullptr);
    lv_obj_set_style_local_bg_color(tensUpBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_size(tensUpBtn, width, btnHeight);
    lv_obj_align(tensUpBtn, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
    tensUpBtn->user_data = this;
    lv_obj_set_event_cb(tensUpBtn, tensUpBtnEventHandler);

    lv_obj_t* tensUpLabel = lv_label_create(tensUpBtn, nullptr);
    lv_obj_set_style_local_text_font(tensUpLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
    lv_label_set_text_static(tensUpLabel, "+");
    lv_obj_align(tensUpLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

    tensDownBtn = lv_btn_create(counterContainer, nullptr);
    lv_obj_set_style_local_bg_color(tensDownBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_size(tensDownBtn, width, btnHeight);
    lv_obj_align(tensDownBtn, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    tensDownBtn->user_data = this;
    lv_obj_set_event_cb(tensDownBtn, tensDownBtnEventHandler);

    lv_obj_t* tensDownLabel = lv_label_create(tensDownBtn, nullptr);
    lv_obj_set_style_local_text_font(tensDownLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
    lv_label_set_text_static(tensDownLabel, "-");
    lv_obj_align(tensDownLabel, nullptr, LV_ALIGN_CENTER, 0, 0);

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

    thousandsUpperLine = LineCreate();
    lv_obj_align(thousandsUpperLine, thousandsUpBtn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    thousandsLowerLine = LineCreate();
    lv_obj_align(thousandsLowerLine, thousandsDownBtn, LV_ALIGN_OUT_TOP_MID, 0, -1);

    hundredsUpperLine = LineCreate();
    lv_obj_align(hundredsUpperLine, hundredsUpBtn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    hundredsLowerLine = LineCreate();
    lv_obj_align(hundredsLowerLine, hundredsDownBtn, LV_ALIGN_OUT_TOP_MID, 0, -1);

    tensUpperLine = LineCreate();
    lv_obj_align(tensUpperLine, tensUpBtn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    tensLowerLine = LineCreate();
    lv_obj_align(tensLowerLine, tensDownBtn, LV_ALIGN_OUT_TOP_MID, 0, -1);
}