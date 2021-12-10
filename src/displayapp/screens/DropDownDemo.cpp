#include "displayapp/screens/DropDownDemo.h"
#include <lvgl/lvgl.h>
#include <libraries/log/nrf_log.h>
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

DropDownDemo::DropDownDemo(Pinetime::Applications::DisplayApp* app) : Screen(app) {
  // Create the dropdown object, with many item, and fix its height
  ddlist = lv_ddlist_create(lv_scr_act(), nullptr);
  lv_ddlist_set_options(ddlist,
                        "Apple\n"
                        "Banana\n"
                        "Orange\n"
                        "Melon\n"
                        "Grape\n"
                        "Raspberry\n"
                        "A\n"
                        "B\n"
                        "C\n"
                        "D\n"
                        "E");
  lv_ddlist_set_fix_width(ddlist, 150);
  lv_ddlist_set_draw_arrow(ddlist, true);
  lv_ddlist_set_fix_height(ddlist, 150);
  lv_obj_align(ddlist, nullptr, LV_ALIGN_IN_TOP_MID, 0, 20);
}

DropDownDemo::~DropDownDemo() {
  // Reset the touchmode
  app->SetTouchMode(DisplayApp::TouchModes::Gestures);
  lv_obj_clean(lv_scr_act());
}

bool DropDownDemo::Refresh() {
  auto* list = static_cast<lv_ddlist_ext_t*>(ddlist->ext_attr);

  // Switch touchmode to Polling if the dropdown is opened. This will allow to scroll inside the
  // dropdown while it is opened.
  // Disable the polling mode when the dropdown is closed to be able to handle the gestures.
  if (list->opened)
    app->SetTouchMode(DisplayApp::TouchModes::Polling);
  else
    app->SetTouchMode(DisplayApp::TouchModes::Gestures);
  return running;
}

bool DropDownDemo::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  // If the dropdown is opened, notify Display app that it doesn't need to handle the event
  // (this will prevent displayApp from going back to the menu or clock scree).
  auto* list = static_cast<lv_ddlist_ext_t*>(ddlist->ext_attr);
  if (list->opened) {
    return true;
  } else {
    return false;
  }
}
