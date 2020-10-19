/*  Copyright (C) 2020 JF, Avamander

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <libs/lvgl/lvgl.h>
#include "Modal.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;

Modal::Modal(Pinetime::Applications::DisplayApp *app) : Screen(app) {


}

Modal::~Modal() {
  lv_obj_clean(lv_scr_act());
}

bool Modal::Refresh() {
  
  return running;
}

bool Modal::OnButtonPushed() {
  running = false;
  return true;
}

void Modal::Hide() {
  /* Delete the parent modal background */
  lv_obj_del_async(lv_obj_get_parent(mbox));
  mbox = NULL; /* happens before object is actually deleted! */
  isVisible = false;
}

void Modal::mbox_event_cb(lv_obj_t *obj, lv_event_t evt) {
  auto* m = static_cast<Modal *>(obj->user_data);
  m->OnEvent(obj, evt);
}

void Modal::OnEvent(lv_obj_t *event_obj, lv_event_t evt) {
  if(evt == LV_EVENT_DELETE && event_obj == mbox) {
    Hide();
  } else if(evt == LV_EVENT_VALUE_CHANGED) {
    /* A button was clicked */
    lv_msgbox_start_auto_close(mbox, 0);
//    Hide();
  }
}

void Modal::Show(const char* msg) {
  if (isVisible) { return; }
  isVisible = true;
  lv_style_copy(&modal_style, &lv_style_plain_color);
  lv_style_set_value_color(&modal_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_bg_grad_color(&modal_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_style_set_value_opa(&modal_style, LV_STATE_DEFAULT, LV_OPA_50);
  
  obj = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style(obj, &modal_style);
  lv_obj_set_pos(obj, 0, 0);
  lv_obj_set_size(obj, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_opa_scale_enable(obj, true); /* Enable opacity scaling for the animation */
  
  static const char* btns2[] = {"Ok", ""};
  
  /* Create the message box as a child of the modal background */
  mbox = lv_msgbox_create(obj, nullptr);
  lv_msgbox_add_btns(mbox, btns2);
  lv_msgbox_set_text(mbox, msg);
  lv_obj_align(mbox, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_event_cb(mbox, Modal::mbox_event_cb);
  
  mbox->user_data = this;
  
  /* Fade the message box in with an animation */
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_time(&a, 500);
  lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
  lv_anim_set_exec_cb(&a, obj, (lv_anim_exec_xcb_t)lv_obj_set_opa_scale);
  lv_anim_create(&a);
}
