#include "displayapp/screens/Dice.h"

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

namespace {
  lv_obj_t* MakeLabel(lv_font_t* font,
                      lv_color_t color,
                      lv_label_long_mode_t longMode,
                      uint8_t width,
                      lv_label_align_t labelAlignment,
                      const char* text,
                      lv_obj_t* reference,
                      lv_align_t alignment,
                      int8_t x,
                      int8_t y) {
    lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
    lv_label_set_long_mode(label, longMode);
    if (width != 0) {
      lv_obj_set_width(label, width);
    }
    lv_label_set_align(label, labelAlignment);
    lv_label_set_text(label, text);
    lv_obj_align(label, reference, alignment, x, y);
    return label;
  }

  static void btnRollEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Dice*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->Roll();
    }
  }
}

Dice::Dice(Controllers::MotionController& motion, Controllers::MotorController& motor) : motor {motor} {
  std::seed_seq sseq {static_cast<uint32_t>(xTaskGetTickCount()),
                      static_cast<uint32_t>(motion.X()),
                      static_cast<uint32_t>(motion.Y()),
                      static_cast<uint32_t>(motion.Z())};
  gen.seed(sseq);

  lv_obj_t* nCounterLabel = MakeLabel(&jetbrains_mono_bold_20,
                                      LV_COLOR_WHITE,
                                      LV_LABEL_LONG_EXPAND,
                                      0,
                                      LV_LABEL_ALIGN_CENTER,
                                      "count",
                                      lv_scr_act(),
                                      LV_ALIGN_IN_TOP_LEFT,
                                      0,
                                      0);

  lv_obj_t* dCounterLabel = MakeLabel(&jetbrains_mono_bold_20,
                                      LV_COLOR_WHITE,
                                      LV_LABEL_LONG_EXPAND,
                                      0,
                                      LV_LABEL_ALIGN_CENTER,
                                      "sides",
                                      nCounterLabel,
                                      LV_ALIGN_OUT_RIGHT_MID,
                                      20,
                                      0);

  nCounter.Create();
  lv_obj_align(nCounter.GetObject(), nCounterLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  nCounter.SetValue(1);

  dCounter.Create();
  lv_obj_align(dCounter.GetObject(), dCounterLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  dCounter.SetValue(6);

  std::uniform_int_distribution<> distrib(0, resultColors.size() - 1);
  currentColorIndex = distrib(gen);

  resultTotalLabel = MakeLabel(&jetbrains_mono_42,
                               resultColors[currentColorIndex],
                               LV_LABEL_LONG_BREAK,
                               120,
                               LV_LABEL_ALIGN_CENTER,
                               "",
                               lv_scr_act(),
                               LV_ALIGN_IN_TOP_RIGHT,
                               11,
                               38);
  resultIndividualLabel = MakeLabel(&jetbrains_mono_bold_20,
                                    resultColors[currentColorIndex],
                                    LV_LABEL_LONG_BREAK,
                                    90,
                                    LV_LABEL_ALIGN_CENTER,
                                    "",
                                    resultTotalLabel,
                                    LV_ALIGN_OUT_BOTTOM_MID,
                                    0,
                                    10);

  Roll();
  openingRoll = false;

  btnRoll = lv_btn_create(lv_scr_act(), nullptr);
  btnRoll->user_data = this;
  lv_obj_set_event_cb(btnRoll, btnRollEventHandler);
  lv_obj_set_size(btnRoll, 240, 50);
  lv_obj_align(btnRoll, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  btnRollLabel = MakeLabel(&jetbrains_mono_bold_20,
                           LV_COLOR_WHITE,
                           LV_LABEL_LONG_EXPAND,
                           0,
                           LV_LABEL_ALIGN_CENTER,
                           Symbols::dice,
                           btnRoll,
                           LV_ALIGN_CENTER,
                           0,
                           0);
}

Dice::~Dice() {
  lv_obj_clean(lv_scr_act());
}

void Dice::Roll() {
  uint8_t resultIndividual;
  uint16_t resultTotal = 0;
  std::uniform_int_distribution<> distrib(1, dCounter.GetValue());

  lv_label_set_text(resultIndividualLabel, "");

  if (nCounter.GetValue() == 1) {
    resultTotal = distrib(gen);
    if (dCounter.GetValue() == 2) {
      switch (resultTotal) {
        case 1:
          lv_label_set_text(resultIndividualLabel, "HEADS");
          break;
        case 2:
          lv_label_set_text(resultIndividualLabel, "TAILS");
          break;
      }
    }
  } else {
    for (uint8_t i = 0; i < nCounter.GetValue(); i++) {
      resultIndividual = distrib(gen);
      resultTotal += resultIndividual;
      lv_label_ins_text(resultIndividualLabel, LV_LABEL_POS_LAST, std::to_string(resultIndividual).c_str());
      if (i < (nCounter.GetValue() - 1)) {
        lv_label_ins_text(resultIndividualLabel, LV_LABEL_POS_LAST, "+");
      }
    }
  }

  lv_label_set_text_fmt(resultTotalLabel, "%d", resultTotal);
  if (openingRoll == false) {
    motor.RunForDuration(30);
    NextColor();
  }
}

void Dice::NextColor() {
  currentColorIndex = (currentColorIndex + 1) % resultColors.size();
  lv_obj_set_style_local_text_color(resultTotalLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, resultColors[currentColorIndex]);
  lv_obj_set_style_local_text_color(resultIndividualLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, resultColors[currentColorIndex]);
}
