#include "displayapp/screens/Dice.h"

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static void btnRollEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto* screen = static_cast<Dice*>(obj->user_data);
  if (event == LV_EVENT_CLICKED) {
    screen->Roll();
  }
}

Dice::Dice(DisplayApp* app) : Screen(app) {
  srand(xTaskGetTickCount() % (std::numeric_limits<unsigned int>::max()));

  nCounter.Create();
  lv_obj_align(nCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 24);
  nCounter.SetValue(1);

  lv_obj_t* dLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(dLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_color(dLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(dLabel, "d");
  lv_obj_align(dLabel, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 57, 80);

  dCounter.Create();
  lv_obj_align(dCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 83, 24);
  dCounter.SetValue(6);

  currentColorIndex = rand() % resultColors.size();

  resultTotalLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(resultTotalLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_color(resultTotalLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, resultColors[currentColorIndex]);
  lv_label_set_long_mode(resultTotalLabel, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(resultTotalLabel, 120);
  lv_label_set_align(resultTotalLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(resultTotalLabel, nullptr, LV_ALIGN_IN_TOP_RIGHT, 11, 25);

  resultIndividualLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(resultIndividualLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(resultIndividualLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, resultColors[currentColorIndex]);
  lv_label_set_long_mode(resultIndividualLabel, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(resultIndividualLabel, 90);
  lv_label_set_align(resultIndividualLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(resultIndividualLabel, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 75);

  Roll();

  btnRoll = lv_btn_create(lv_scr_act(), nullptr);
  btnRoll->user_data = this;
  lv_obj_set_event_cb(btnRoll, btnRollEventHandler);
  lv_obj_set_size(btnRoll, 240, 50);
  lv_obj_align(btnRoll, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  btnRollLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(btnRollLabel, btnRoll, LV_ALIGN_CENTER, 12, 0);
  lv_label_set_text_static(btnRollLabel, Symbols::dice);
}

Dice::~Dice() {
  lv_obj_clean(lv_scr_act());
}

void Dice::Roll() {
  uint8_t resultIndividual;
  uint16_t resultTotal = 0;

  lv_label_set_text(resultIndividualLabel, "");

  for (uint8_t i = 0; i < nCounter.GetValue(); i++) {
    resultIndividual = ((rand() % dCounter.GetValue()) + 1);
    resultTotal += resultIndividual;
    lv_label_ins_text(resultIndividualLabel, LV_LABEL_POS_LAST, std::to_string(resultIndividual).c_str());
    if (i < (nCounter.GetValue() - 1)) {
      lv_label_ins_text(resultIndividualLabel, LV_LABEL_POS_LAST, "+");
    }
  }

  lv_label_set_text_fmt(resultTotalLabel, "%d", resultTotal);
  NextColor();
}

void Dice::NextColor() {
  currentColorIndex = (currentColorIndex + 1) % resultColors.size();
  lv_obj_set_style_local_text_color(resultTotalLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, resultColors[currentColorIndex]);
  lv_obj_set_style_local_text_color(resultIndividualLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, resultColors[currentColorIndex]);
}
