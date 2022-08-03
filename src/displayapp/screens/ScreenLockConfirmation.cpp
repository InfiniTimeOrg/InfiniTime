#include "displayapp/screens/ScreenLockConfirmation.h"

#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
      static_cast<ScreenLockConfirmation*>(obj->user_data)->OnButtonEvent(obj);
    }
  }
}

ScreenLockConfirmation::ScreenLockConfirmation(DisplayApp* app, Controllers::TouchHandler& touchHandler)
  : app {app}, touchHandler {touchHandler} {
  label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(label, true);
  lv_label_set_text_static(label, "\n#FFFF00 Disable touch?#\n\n\n#808080 Press and hold#\n#808080 button to enable.#");
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  btnEnable = lv_btn_create(lv_scr_act(), nullptr);
  btnEnable->user_data = this;
  lv_obj_set_event_cb(btnEnable, ButtonEventHandler);
  lv_obj_set_size(btnEnable, 115, 50);
  lv_obj_align(btnEnable, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  txtEnable = lv_label_create(btnEnable, nullptr);
  lv_label_set_text_static(txtEnable, "Disable");
}

ScreenLockConfirmation::~ScreenLockConfirmation() {
  lv_obj_clean(lv_scr_act());
}

void ScreenLockConfirmation::OnButtonEvent(lv_obj_t* obj) {
  if (obj == btnEnable) {
    touchHandler.SetScreenLockStatus(true);
    app->StartApp(Apps::Clock, DisplayApp::FullRefreshDirections::LeftAnim);
  }
}
