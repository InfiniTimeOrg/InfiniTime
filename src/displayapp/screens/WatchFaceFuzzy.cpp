#include "WatchFaceFuzzy.h"

#include <lvgl/lvgl.h>
#include <string>

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

  /* Set the behavior of the label when the text is longer than the
   * object size. In this case we want to break the text to keep the
   * width and extend the height of the obj.
   *
   * Note: let's try to use linebreaks explicitly to avoid problems with
   * the hour accent (which tends to break with this long break mode)
   */
  lv_label_set_long_mode(label_time, LV_LABEL_LONG_BREAK);
  /* Set size of the label to screen size */
  //lv_obj_set_size(label_time, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_width(label_time, LV_HOR_RES);

  /* Set text alignment in the label.
   * NOTE: alignment is character-wise and not pixel-wise (the result is
   * probably not what you would expect is un are centering text)
   */
  lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);

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
   *
   * NOTE: Updating every minute is efficient but it means that you need
   * to wait for up to one minute for the clock to update after
   * bluetooth connection (e.g., when you hard reset the PineTime).
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
  uint8_t hours, minutes;
  std::string hoursStr, timeStr;

  hours = dateTimeController.Hours() % 12;  // TODO: maybe that's not needed?
  minutes = dateTimeController.Minutes();
  auto sector = (minutes / 5 + (minutes % 5 > 2)) % 12;

  timeStr = timeSectors[sector];
  if (timeStr.find("%1") != std::string::npos)
    hours = (hours + 1) % 12;
  hoursStr = std::string("#") + timeAccent + " " + hourNames[hours] + "#";
  timeStr.replace(timeStr.find("%"), 2, hoursStr);

  lv_label_set_text_fmt(label_time, timeStr.c_str());

  /* Align the label w.r.t. another object (active screen in this case)
   * You can use the last two parameters to move the obj around 
   * *after the alignment*.
   * See https://docs.lvgl.io/latest/en/html/_images/align.png for all
   * possible alignments.
   *
   * NOTE: you should set the alignment after determining the size
   * (and content) of the label to get a reliable result!
   */
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}

/* Consider something like this
 *  https://salsa.debian.org/xfce-team/desktop/xfce4-panel/-/blob/debian/master/plugins/clock/clock-fuzzy.c
 * to gradually implement internationalisation
 */
const char* WatchFaceFuzzy::timeSectors[] = {
  "%0\no'clock",
  "five past\n%0",
  "ten past\n%0",
  "quarter\npast\n%0",
  "twenty\npast\n%0",
  "twenty\nfive past\n%0",
  "half past\n%0",
  "twenty\nfive to\n%1",
  "twenty\nto %1",
  "quarter\nto %1",
  "ten to\n%1",
  "five to\n%1",
};
const char* WatchFaceFuzzy::hourNames[] = {
  "twelve",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
};

