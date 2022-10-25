#include "displayapp/screens/Morse.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

Morse::Morse(DisplayApp* app) : Screen(app) {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "A. B -... C -.-. D -.. E. F..-. G--. H.... I.. J.--- K-.- L.-.. M-- N-.");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* t2 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(t2, "O--- P.--. Q--.- R.-. S... T- U..- V...- W.-- X-..- Y-.-- Z--..");
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 20, 0);
}

Morse::~Morse() {
  lv_obj_clean(lv_scr_act());
}