#include "displayapp/screens/CoinFlip.h"
#include "displayapp/DisplayApp.h"

#include <cstdlib> // for rand()

using namespace Pinetime::Applications::Screens;

char* flipCoin() {
  int flip;
  char* result;

  flip = rand() % 2 + 1;
  if(flip == 1) {
    result = "Heads";
  } else {
    result = "Tails";
  }

  return result;
}

CoinFlip::CoinFlip(DisplayApp* app) : Screen(app) {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Coin Flip");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  // Where the result is posted
  char* result = flipCoin();
  lv_obj_t* resultText = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(resultText, result);
  lv_label_set_align(resultText, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(resultText, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  // Our 'Flip' button
  lv_obj_t* button = lv_btn_create(lv_scr_act(), nullptr);
  lv_btn_set_text_static(button, "Flip");
  lv_btn_set_align(button, LV_BTN_ALIGN_CENTER);
  lv_obj_align(button, lv_scr_act(), LV_ALIGN_BOTTOM, 0, 0);
}

CoinFlip::~CoinFlip() {
  lv_obj_clean(lv_scr_act());
}
