#include "WatchFaceFuzzy.h"

#include <string>
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

WatchFaceFuzzy::WatchFaceFuzzy(DisplayApp* app,
                               Controllers::DateTime& dateTimeController,
                               Controllers::Settings& settingsController,
                               Controllers::MotorController& motorController,
                               Controllers::MotionController& motionController)
  : Screen(app),
    dateTimeController {dateTimeController},
    settingsController {settingsController},
    motorController {motorController},
    motionController {motionController} {

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  timeLabel = lv_label_create(backgroundLabel, nullptr);
  lv_label_set_long_mode(timeLabel, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(timeLabel, LV_HOR_RES);
  lv_label_set_align(timeLabel, LV_LABEL_ALIGN_CENTER);
  lv_label_set_recolor(timeLabel, true);
  lv_obj_set_style_local_text_color(timeLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_obj_set_style_local_text_font(timeLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  Refresh();
}

WatchFaceFuzzy::~WatchFaceFuzzy() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceFuzzy::Refresh() {
  // Triggered once, on shaking state change (off -> on)
  should_digital = motionController.Should_ShakeWake(settingsController.GetShakeThreshold());
  if (!shaking && should_digital) {
    motorController.RunForDuration(35);
    app->StartApp(Apps::WatchFaceDigitalPreview, DisplayApp::FullRefreshDirections::Right);
  }
  shaking = should_digital;

  // TODO: while the refresh rate is high enough to detect interaction,
  // we only need to run the rest of the code every 60 seconds.
  uint8_t hours, minutes;
  std::string hoursStr, timeStr;

  hours = dateTimeController.Hours() % 12;
  minutes = dateTimeController.Minutes();
  auto sector = minutes / 5 + (minutes % 5 > 2);
  if (sector == 12) {
    hours = (hours + 1) % 12;
    sector = 0;
  }

  timeStr = timeSectors[sector];
  if (timeStr.find("%1") != std::string::npos) {
    hours = (hours + 1) % 12;
  }
  hoursStr = std::string("#") + timeAccent + " " + hourNames[hours] + "#";
  timeStr.replace(timeStr.find("%"), 2, hoursStr);

  lv_label_set_text(timeLabel, timeStr.c_str());
  lv_obj_align(timeLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
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
 * const char* ca-ES_sectors[] = {
 *   "%0\nen punt",
 *   "%0\ni cinc",
 *   "%0\ni deu",
 *   "%0\ni quart",
 *   "%0\ni vint",
 *   "%0\ni vint-\ni-cinc",
 *   "%0\ni mitja",
 *   "%1\nmenys\nvint-\ni-cinc",
 *   "%1\nmenys\nvint",
 *   "%1\nmenys\nquart",
 *   "%1\nmenys deu",
 *   "%1\nmenys\ncinc",
 * };
 * const char* ca-ES_hourNames[] = {
 *   "les dotze",
 *   "la una",
 *   "les dues",
 *   "les tres",
 *   "les\nquatre",
 *   "les cinc",
 *   "les sis",
 *   "les set",
 *   "les vuit",
 *   "les nou",
 *   "les deu",
 *   "les onze",
 * };
 *
 * const char* es-ES_sectors[] = {
 *   "%0\nen punto",
 *   "%0\ny cinco",
 *   "%0\ny diez",
 *   "%0\ny cuarto",
 *   "%0\ny veinte",
 *   "%0\ny veinti\ncinco",
 *   "%0\ny media",
 *   "%1\nmenos\nveinti\ncinco",
 *   "%1\nmenos\nveinte",
 *   "%1\nmenos\ncuarto",
 *   "%1\nmenos\ndiez",
 *   "%1\nmenos\ncinco",
 * };
 * const char* es-ES_hourNames[] = {
 *   "las doce",
 *   "la una",
 *   "las dos",
 *   "las tres",
 *   "las\ncuatro",
 *   "las cinco",
 *   "las seis",
 *   "las siete",
 *   "las ocho",
 *   "las nueve",
 *   "las diez",
 *   "las once",
 * };
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
 *
 * const char* de_DE_sectors[] = {
 *   "%0 Uhr",
 *   "Fünf nach %0",
 *   "Zehn nach %0",
 *   "Viertel nach %0",
 *   "Zwanzig nach %0",
 *   "Fünf vor halb %1",
 *   "Halb %1",
 *   "Fünf nach halb %1",
 *   "Zwanzig vor %1",
 *   "Viertel vor %1",
 *   "Zehn vor %1",
 *   "Fünf vor %1",
 * };
 * const char* de_DE_hourNames[] = {
 *   "Zwölf",
 *   "Eins", // TODO: "Ein" in "Ein Uhr"
 *   "Zwei",
 *   "Drei",
 *   "Vier",
 *   "Fünf",
 *   "Sechs",
 *   "Sieben",
 *   "Acht",
 *   "Neun",
 *   "Zehn",
 *   "Elf",
 * };
 */
