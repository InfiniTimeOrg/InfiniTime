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

Dice::Dice(DisplayApp* app, Controllers::DateTime& dateTime) : Screen(app) {

  srand(dateTime.Uptime().count() * time(nullptr));

  lv_obj_t* dLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(dLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_set_style_local_text_color(dLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(dLabel, "d");
  lv_obj_align(dLabel, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 14, 80);

  sidesCounter.Create();
  lv_obj_align(sidesCounter.GetObject(), nullptr, LV_ALIGN_IN_TOP_LEFT, 49, 24);
  sidesCounter.SetValue(2);

  currentColorIndex = rand() % resultColors.size();

  resultLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(resultLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(resultLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, resultColors[currentColorIndex]);
  lv_obj_align(resultLabel, nullptr, LV_ALIGN_IN_TOP_RIGHT, -110, 60);

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
  rollResult = ((rand() % sidesCounter.GetValue()) + 1);
  lv_label_set_text_fmt(resultLabel, "%d", rollResult);
  NextColor();
}

void Dice::NextColor() {
  currentColorIndex = (currentColorIndex + 1) % resultColors.size();
  lv_obj_set_style_local_text_color(resultLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, resultColors[currentColorIndex]);
}
