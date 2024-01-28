#include "displayapp/widgets/PopupMessage.h"
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Applications::Widgets;

PopupMessage::PopupMessage(const char* msg) : message {msg} {
}

void PopupMessage::Create() {
  btnPopup = lv_btn_create(lv_scr_act(), nullptr);
  btnPopup->user_data = this;
  lv_obj_set_size(btnPopup, 200, 150);
  lv_obj_align(btnPopup, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(btnPopup, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_60);
  lv_obj_t* lblMessage = lv_label_create(btnPopup, nullptr);
  lv_label_set_text_static(lblMessage, message);
  lv_obj_set_hidden(btnPopup, isHidden);
}

void PopupMessage::SetHidden(bool hidden) {
  if (isHidden == hidden) {
    return;
  }
  isHidden = hidden;
  // create/delete on demand
  if (btnPopup == nullptr && !isHidden) {
    Create();
  } else if (btnPopup != nullptr) {
    lv_obj_del(btnPopup);
    btnPopup = nullptr;
  }
}

bool PopupMessage::IsHidden() {
  return isHidden;
}
