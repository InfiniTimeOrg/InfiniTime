#include "WatchFaceFuzzy.h"

//#include <date/date.h>
#include <lvgl/lvgl.h>
//#include <cstdio>

using namespace Pinetime::Applications::Screens;

WatchFaceFuzzy::WatchFaceFuzzy(DisplayApp* app, Controllers::DateTime& dateTimeController)
  : Screen(app), dateTimeController {dateTimeController} {

  /* Why would we need this? */
  //settingsController.SetClockFace(0);

  /*
   * `lv_scr_act()` get the active screen on the default display
   * (note that we only have one display in this setup, but LVGL has
   * multidisplay support as well).
   *
   * `nullptr` is used simply because we don't want to *copy* the label
   * from any other object.
   */
  label_time = lv_label_create(lv_scr_act(), nullptr);

  /* Set size of the label to screen size */
  lv_obj_set_size(label_time, LV_HOR_RES, LV_VER_RES);

  /* Set the behavior of the label when the text is longer than the
   * object size. In this case we want to break the text to keep the
   * width and extend the height of the obj.
   *
   * Note: this doesn't work properly or it is someone dependent on when
   * it is called. Avoiding it for now and specifying linebreaks
   * directly in the string.
   */
  //lv_label_set_long_mode(label_time, LV_LABEL_LONG_BREAK);

  /* Set text alignment (left) in the label */
  lv_label_set_align(label_time, LV_LABEL_ALIGN_LEFT);

  /* Set main color and font for the label
   * Note: color can be changed when setting the text as well allowing
   * for multicolor strings. Can we do this with fonts as well?
   *
   * "local" style take precedence over global styles
   *
   * `LV_LABEL_PART_MAIN` is the only "part" of a label.
   * 
   * `LV_STATE_DEFAULT` is the label state (which cannot be checked or
   * toggled or whatever...).
   */
  lv_label_set_recolor(label_time, true);
  lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x777777));
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_regular_36);

  /* Creates a new LVGL task called periodically.
   *    lv_task_create(callback, period, priority, userdata)
   * callback: in this case is a generic wrapper of Refresh()
   * period: how often the callback is triggered (every 60s == 60000ms)
   * priority: see LVGL for more info
   * userdata: accessible from the callback (see Screen.cpp)
   */
  taskRefresh = lv_task_create(RefreshTaskCallback, 60000, LV_TASK_PRIO_MID, this);

  /* Refresh to avoid waiting for 60s */
  Refresh();
}

WatchFaceFuzzy::~WatchFaceFuzzy() {
  /* Remove refresh task for this watchface */
  lv_task_del(taskRefresh);
  /* Delete all children of the active screen (this watchface) */
  lv_obj_clean(lv_scr_act());
}

void WatchFaceFuzzy::Refresh() {
  /* Possible wordings:
   * - "<hours> o'clock"
   * - "half past <hours>"
   * - "quarter past/to <hours>"
   * - "<min_tens> past/to <hours>"
   * - "<min_units> past/to <hours>"
   * - "<min_tens> <min_units> past/to <hours>"
   */
  uint8_t hours, minutes, minutesUnits;
  char const* pastTo = "past";
  char const* hoursAccent = "ffffff";
  char const* strings[20] = { "one", "two", "three", "four", "five",
    "six", "seven", "eight", "nine", "ten", "eleven", "twelve",
    "thirteen", "fourteen", "quarter", "seventeen", "eighteen", "nineteen",
    "twenty" };
  hours = dateTimeController.Hours() % 12;
  minutes = dateTimeController.Minutes();
  if (minutes > 30) {
    pastTo = "to";
    hours = (hours + 1) % 12;
    minutes = 60 - minutes;
  }
  minutesUnits = minutes % 10;
  auto hoursStr = strings[(hours + 11) % 12];
  if (minutes == 0) {
    lv_label_set_text_fmt(label_time, "#%s %s#\no'clock", hoursAccent, hoursStr);
  } else if (minutes == 30) {
    lv_label_set_text_fmt(label_time, "half\npast\n#%s %s#", hoursAccent, hoursStr);
  } else if (minutes <= 20) {
    lv_label_set_text_fmt(label_time, "%s\n%s\n#%s %s#", strings[minutes - 1], pastTo, hoursAccent, hoursStr);
  } else {
    lv_label_set_text_fmt(label_time, "twenty\n%s %s\n#%s %s#", strings[minutesUnits - 1], pastTo, hoursAccent, hoursStr);
  }
  /* Align the label w.r.t. another object (active screen in this case)
   * You can use the last two parameters to move the obj around 
   * *after the alignment*.
   * See https://docs.lvgl.io/latest/en/html/_images/align.png for all
   * possible alignments.
   *
   * NOTE: you should set the alignment after determining the size
   * (and content) of the label to get a reliable result!
   */
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 10, -10);
}
