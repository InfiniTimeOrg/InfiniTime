#include "SimpleAlert.h"

using namespace Pinetime::Applications::Screens::Alert;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;

SimpleAlert::SimpleAlert(Pinetime::Applications::DisplayApp *app) : Modal(app) {}

SimpleAlert::~SimpleAlert() {
  lv_obj_clean(lv_scr_act());
}

void SimpleAlert::OnEvent(lv_obj_t *event_obj, lv_event_t evt) {
  if(evt == LV_EVENT_DELETE && event_obj == mbox) {
    Hide();
  } else if(evt == LV_EVENT_VALUE_CHANGED) {
    if(event_obj == mbox) {
      if(strcmp(lv_mbox_get_active_btn_text(event_obj), BTN_OK.c_str()) == 0) {
        lv_mbox_start_auto_close(mbox, 0);
      }
    }
  }
}

void SimpleAlert::Show(struct Pinetime::Controllers::NotificationManager::Notification notification) {
  if(isVisible) return;
  isVisible = true;
  lv_style_copy(&modal_style, &lv_style_plain_color);
  modal_style.body.main_color = modal_style.body.grad_color = LV_COLOR_BLACK;
  modal_style.body.opa = LV_OPA_50;

  obj = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style(obj, &modal_style);
  lv_obj_set_pos(obj, 0, 0);
  lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_opa_scale_enable(obj, true); /* Enable opacity scaling for the animation */

  static const char * btns2[] = {
    BTN_OK.c_str(),
    ""
  };

  /* Create the message box as a child of the modal background */
  mbox = lv_mbox_create(obj, nullptr);
  lv_mbox_add_btns(mbox, btns2);
  lv_mbox_set_text(mbox, notification.message.data());
  lv_obj_align(mbox, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_event_cb(mbox, Modal::mbox_event_cb);

  mbox->user_data = this;

  /* Fade the message box in with an animation */
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_time(&a, 500, 0);
  lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
  lv_anim_set_exec_cb(&a, obj, (lv_anim_exec_xcb_t)lv_obj_set_opa_scale);
  lv_anim_create(&a);
}
