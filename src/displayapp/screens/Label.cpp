#include "displayapp/screens/Label.h"

using namespace Pinetime::Applications::Screens;

Label::Label(uint8_t screenID, uint8_t numScreens, Pinetime::Applications::DisplayApp* app, lv_obj_t* labelText)
  : Screen(app), labelText {labelText}, pageIndicator(screenID, numScreens) {

  pageIndicator.Create();
}

Label::~Label() {
  lv_obj_clean(lv_scr_act());
}
