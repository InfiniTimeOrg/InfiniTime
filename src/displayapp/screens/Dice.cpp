#include "Dice.h"

#include "Screen.h"
#include "Symbols.h"
#include "lvgl/lvgl.h"

using namespace Pinetime::Applications::Screens;
constexpr uint8_t Dice::diceSizes[];

static void num_dice_inc_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto* dice = static_cast<Dice*>(obj->user_data);
  dice->numDiceIncBtnEventHandler(event);
}

static void num_dice_dec_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto* dice = static_cast<Dice*>(obj->user_data);
  dice->numDiceDecBtnEventHandler(event);
}

static void dice_size_inc_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto* dice = static_cast<Dice*>(obj->user_data);
  dice->diceSizeIncBtnEventHandler(event);
}

static void dice_size_dec_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto* dice = static_cast<Dice*>(obj->user_data);
  dice->diceSizeDecBtnEventHandler(event);
}

static void roll_event_handler(lv_obj_t* obj, lv_event_t event) {
  auto* dice = static_cast<Dice*>(obj->user_data);
  dice->rollBtnEventHandler(event);
}

void Dice::createScreen() {
  resultCont = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(resultCont, 240, 80);
  lv_obj_set_pos(resultCont, 0, 0);
  lv_cont_set_layout(resultCont, LV_LAYOUT_COLUMN_LEFT);
  lv_obj_set_style_local_pad_all(resultCont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

  resultTotalLabel = lv_label_create(resultCont, nullptr);
  lv_obj_set_size(resultTotalLabel, 220, 30);
  lv_obj_set_style_local_text_color(resultTotalLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text_static(resultTotalLabel, "");

  resultIndRollsLabel = lv_label_create(resultCont, nullptr);
  lv_label_set_long_mode(resultIndRollsLabel, LV_LABEL_LONG_SROLL_CIRC);
  lv_obj_set_size(resultIndRollsLabel, 220, 30);
  lv_obj_set_style_local_text_color(resultIndRollsLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_label_set_text_static(resultIndRollsLabel, "");

  configCont = lv_cont_create(lv_scr_act(), nullptr);
  lv_obj_set_size(configCont, 240, 154);
  lv_obj_set_pos(configCont, 0, 86);
  lv_obj_set_style_local_bg_color(configCont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  numDiceIncBtn = lv_btn_create(configCont, nullptr);
  numDiceIncBtn->user_data = this;
  lv_obj_set_event_cb(numDiceIncBtn, num_dice_inc_event_handler);
  lv_obj_set_size(numDiceIncBtn, 76, 52);
  lv_obj_align(numDiceIncBtn, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_style_local_value_str(numDiceIncBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");

  numDiceLabelCont = lv_cont_create(configCont, nullptr);
  lv_obj_set_size(numDiceLabelCont, 76, 50);
  lv_obj_align(numDiceLabelCont, nullptr, LV_ALIGN_IN_LEFT_MID, 0, 0);
  lv_obj_set_style_local_bg_color(numDiceLabelCont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_cont_set_layout(numDiceLabelCont, LV_LAYOUT_CENTER);
  numDiceLabel = lv_label_create(numDiceLabelCont, nullptr);
  lv_label_set_text_static(numDiceLabel, "1");

  numDiceDecBtn = lv_btn_create(configCont, nullptr);
  numDiceDecBtn->user_data = this;
  lv_obj_set_event_cb(numDiceDecBtn, num_dice_dec_event_handler);
  lv_obj_set_size(numDiceDecBtn, 76, 52);
  lv_obj_align(numDiceDecBtn, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_value_str(numDiceDecBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");

  diceSizeIncBtn = lv_btn_create(configCont, nullptr);
  diceSizeIncBtn->user_data = this;
  lv_obj_set_event_cb(diceSizeIncBtn, dice_size_inc_event_handler);
  lv_obj_set_size(diceSizeIncBtn, 76, 52);
  lv_obj_align(diceSizeIncBtn, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);
  lv_obj_set_style_local_value_str(diceSizeIncBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "+");

  diceSizeLabelCont = lv_cont_create(configCont, nullptr);
  lv_obj_set_size(diceSizeLabelCont, 76, 50);
  lv_obj_align(diceSizeLabelCont, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_color(diceSizeLabelCont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_cont_set_layout(diceSizeLabelCont, LV_LAYOUT_CENTER);
  diceSizeLabel = lv_label_create(diceSizeLabelCont, nullptr);
  lv_label_set_text_static(diceSizeLabel, "d6");

  diceSizeDecBtn = lv_btn_create(configCont, nullptr);
  diceSizeDecBtn->user_data = this;
  lv_obj_set_event_cb(diceSizeDecBtn, dice_size_dec_event_handler);
  lv_obj_set_size(diceSizeDecBtn, 76, 52);
  lv_obj_align(diceSizeDecBtn, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_local_value_str(diceSizeDecBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "-");

  rollBtn = lv_btn_create(configCont, nullptr);
  rollBtn->user_data = this;
  lv_obj_set_event_cb(rollBtn, roll_event_handler);
  lv_obj_set_size(rollBtn, 76, 154);
  lv_obj_align(rollBtn, nullptr, LV_ALIGN_IN_RIGHT_MID, 0, 0);
  lv_obj_set_style_local_value_str(rollBtn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, "Roll");
}

void Dice::rollDice() {
  uint8_t roll = 0;
  uint16_t result = 0;

  lv_label_set_text(resultIndRollsLabel, "");

  for (int i = 0; i < numDice; i++) {
    roll = ble_ll_rand() % diceSizes[diceSizeIndex] + 1;
    result += roll;

    if (numDice > 1) {
      if (i == 0) {
        lv_label_ins_text(resultIndRollsLabel, 0, "(");
        lv_label_ins_text(resultIndRollsLabel, LV_LABEL_POS_LAST, std::to_string(roll).c_str());
        lv_label_ins_text(resultIndRollsLabel, LV_LABEL_POS_LAST, ",");
      } else if (i < numDice - 1) {
        lv_label_ins_text(resultIndRollsLabel, LV_LABEL_POS_LAST, std::to_string(roll).c_str());
        lv_label_ins_text(resultIndRollsLabel, LV_LABEL_POS_LAST, ",");
      } else {
        lv_label_ins_text(resultIndRollsLabel, LV_LABEL_POS_LAST, std::to_string(roll).c_str());
        lv_label_ins_text(resultIndRollsLabel, LV_LABEL_POS_LAST, ")");
      }
    }
  }

  lv_label_set_text_fmt(resultTotalLabel, "%d", result);
}

Dice::Dice(DisplayApp* app,
           Controllers::MotorController& motorController,
           System::SystemTask& systemTask)
  : Screen(app), systemTask {systemTask}, motorController {motorController} {

  createScreen();
}

Dice::~Dice() {
  lv_obj_clean(lv_scr_act());
}

void Dice::numDiceIncBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  if (numDice < 99) {
    numDice++;
    lv_label_set_text_fmt(numDiceLabel, "%d", numDice);
  }
}

void Dice::numDiceDecBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  if (numDice > 1) {
    numDice--;
    lv_label_set_text_fmt(numDiceLabel, "%d", numDice);
  }
}

void Dice::diceSizeIncBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  if (diceSizeIndex < (sizeof(diceSizes) / sizeof(diceSizes[0])) - 1) {
    diceSizeIndex++;
    lv_label_set_text_fmt(diceSizeLabel, "d%d", diceSizes[diceSizeIndex]);
  }
}

void Dice::diceSizeDecBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  if (diceSizeIndex > 0) {
    diceSizeIndex--;
    lv_label_set_text_fmt(diceSizeLabel, "d%d", diceSizes[diceSizeIndex]);
  }
}

void Dice::rollBtnEventHandler(lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  motorController.RunForDuration(15);

  rollDice();
}
