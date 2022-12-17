#include "displayapp/screens/DotLabel.h"

using namespace Pinetime::Applications::Screens;

DotLabel::DotLabel(uint8_t screenID, uint8_t numScreens, Pinetime::Applications::DisplayApp* app, lv_obj_t* labelText)
  : Screen(app), labelText {labelText}, dotIndicator(screenID, numScreens) {

  dotIndicator.Create();
}

DotLabel::~DotLabel() {
  lv_obj_clean(lv_scr_act());
}
