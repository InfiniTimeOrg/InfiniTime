#include "WatchFaceFuzzy.h"

#include <lvgl/lvgl.h>
#include <string>

using namespace Pinetime::Applications::Screens;

WatchFaceFuzzy::WatchFaceFuzzy(DisplayApp* app, Controllers::DateTime& dateTimeController)
  : Screen(app), dateTimeController {dateTimeController} {

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(label_time, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(label_time, LV_HOR_RES);
  lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
  lv_label_set_recolor(label_time, true);
  lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x777777));
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_regular_36);

  taskRefresh = lv_task_create(RefreshTaskCallback, 60000, LV_TASK_PRIO_MID, this);

  Refresh();
}

WatchFaceFuzzy::~WatchFaceFuzzy() {
  lv_task_del(taskRefresh);
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

  lv_label_set_text(label_time, timeStr.c_str());
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}

/* Inspired by XFCE4-panel's fuzzy clock.
 *
 *      https://salsa.debian.org/xfce-team/desktop/xfce4-panel/-/blob/debian/master/plugins/clock/clock-fuzzy.c
 *
 * Strings contain either a `%0` or a `%1`, indicating the position of
 * the `hour` or `hour+1`, respectively.
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

/* Once i18n is implemented, new languages can be introduced like this:
 *
 * char* it-IT_sectors[] = {
 *   "%0\nin punto",
 *   "%0 e cinque",
 *   "%0 e dieci",
 *   "%0 e un quarto",
 *   "%0 e venti",
 *   "%0 e venti cinque",
 *   "%0 e mezza",
 *   "%0 e trenta cinque",
 *   "%1 meno venti",
 *   "%1 meno un quarto",
 *   "%1 meno dieci",
 *   "%1 meno cinque",
 * };
 * const char* it-IT_hourNames[] = {
 *   "dodici",
 *   "una",
 *   "due",
 *   "tre",
 *   "quattro",
 *   "cinque",
 *   "sei",
 *   "sette",
 *   "otto",
 *   "nove",
 *   "dieci",
 *   "undici",
 * };
 */