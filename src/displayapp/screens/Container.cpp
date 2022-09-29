#include "displayapp/screens/Container.h"

using namespace Pinetime::Applications::Screens;

Container::Container(
    DisplayApp* app,
    lv_obj_t* container,
    uint8_t screenIdx,
    uint8_t nScreens,
    bool horizontal)
  : Screen(app), container {container}, pageIndicator {screenIdx, nScreens, horizontal} {
    pageIndicator.Create();
}

Container::~Container() {
  lv_obj_clean(lv_scr_act());
}
