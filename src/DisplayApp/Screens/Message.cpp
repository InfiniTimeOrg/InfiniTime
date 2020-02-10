#include <cstdio>
#include <libs/date/includes/date/date.h>
#include <Components/DateTime/DateTimeController.h>
#include <Version.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <libs/lvgl/src/lv_font/lv_font.h>
#include <libs/lvgl/lvgl.h>
#include "Message.h"

using namespace Pinetime::Applications::Screens;

extern lv_font_t jetbrains_mono_extrabold_compressedextrabold_compressed;

lv_obj_t * label;
int x = 0;
void Message::Refresh(bool fullRefresh) {
  if(fullRefresh) {
    label = lv_label_create(lv_scr_act(), NULL);          /*Add a label to the button*/
    labelStyle = const_cast<lv_style_t *>(lv_label_get_style(label, LV_LABEL_STYLE_MAIN));
    labelStyle->text.font = &jetbrains_mono_extrabold_compressedextrabold_compressed;
    lv_label_set_style(label, LV_LABEL_STYLE_MAIN, labelStyle);
    lv_label_set_text(label, "01:23");                     /*Set the labels text*/
  } else {
    lv_obj_set_pos(label, 0, x++);
    if(x > 200) x = 0;
  }

}
