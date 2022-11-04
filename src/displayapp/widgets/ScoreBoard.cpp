#include "displayapp/widgets/ScoreBoard.h"

using namespace Pinetime::Applications::Widgets;

ScoreBoard::ScoreBoard() {
}

void ScoreBoard::SetTopText(char *text) {
    lv_label_set_text_static(topLabel, text);
}

void ScoreBoard::SetBottomText(char *text) {
    lv_label_set_text_static(bottomLabel, text);
}

void ScoreBoard::SetTopText(uint8_t score) {
    lv_label_set_text_fmt(topLabel, "%.*i", true, score);
}

void ScoreBoard::SetBottomText(uint8_t score) {
    lv_label_set_text_fmt(bottomLabel, "%.*i", true, score);
}

lv_obj_t* ScoreBoard::Create(char *title) {
    counterContainer = lv_obj_create(lv_scr_act(), nullptr);
    topContainer = lv_obj_create(counterContainer, nullptr);
    bottomContainer = lv_obj_create(counterContainer, nullptr);

    int containerHeight = (fullHeight-titleHeight)/2;
    lv_obj_set_size(counterContainer, width, fullHeight);
    lv_obj_set_size(topContainer, width, containerHeight);
    lv_obj_set_size(bottomContainer, width, containerHeight);

    lv_obj_align(topContainer, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_align(bottomContainer, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    lv_obj_set_style_local_bg_color(counterContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_color(topContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_bg_color(bottomContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

    lv_obj_t* titleLable = lv_label_create(counterContainer, nullptr);
    lv_obj_align(titleLable, nullptr, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(titleLable, true);

    topLabel = lv_label_create(topContainer, nullptr);
    lv_obj_set_style_local_text_font(topLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
    lv_obj_align(topLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(topLabel, true);

    bottomLabel = lv_label_create(bottomContainer, nullptr);
    lv_obj_set_style_local_text_font(bottomLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
    lv_obj_align(bottomLabel, nullptr, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(bottomLabel, true);

    lv_label_set_text_static(titleLable, title);
    lv_label_set_text_static(topLabel, "0");
    lv_label_set_text_static(bottomLabel, "0");

    return counterContainer;
}

void ScoreBoard::SetColor(lv_color_t color) {
    lv_obj_set_style_local_bg_color(topContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, color);
    lv_obj_set_style_local_bg_color(bottomContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, color);
}
