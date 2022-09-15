#include "displayapp/screens/WatchFaceAccurateWords.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

WatchFaceAccurateWords::WatchFaceAccurateWords(DisplayApp* app,
                                               Controllers::DateTime& dateTimeController,
                                               Controllers::Battery& batteryController,
                                               Controllers::Ble& bleController,
                                               Controllers::NotificationManager& notificatioManager,
                                               Controllers::Settings& settingsController,
                                               Controllers::HeartRateController& heartRateController,
                                               Controllers::MotionController& motionController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    statusIcons(batteryController, bleController) {

  statusIcons.Create();

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  label_date = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_label_set_long_mode(label_date, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(label_date, LV_HOR_RES - 20);
  lv_label_set_align(label_date, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_label_set_long_mode(label_time, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(label_time, LV_HOR_RES - 20);
  lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FFE7));
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceAccurateWords::~WatchFaceAccurateWords() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceAccurateWords::Refresh() {
  statusIcons.Update();

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = dateTimeController.CurrentDateTime();

  if (currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime - dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto year = static_cast<int>(yearMonthDay.year());
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>(static_cast<unsigned>(yearMonthDay.month()));
    auto day = static_cast<unsigned>(yearMonthDay.day());
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    uint8_t hour = time.hours().count();
    uint8_t minute = time.minutes().count();

    uint8_t hour_adjusted;
    char words[78];
    char part_day[20];
    const char* hour_word_array[26] = {"midnight", "one",   "two",    "three",  "four", "five",   "six",     "seven", "eight",
                                       "nine",     "ten",   "eleven", "twelve", "one",  "two",    "three",   "four",  "five",
                                       "six",      "seven", "eight",  "nine",   "ten",  "eleven", "midnight"};
    const char* part_day_word_array[9] = {" at night",
                                          " in the early hours",
                                          " in the morning",
                                          " in the morning",
                                          " in the afternoon",
                                          " in the afternoon",
                                          " in the evening",
                                          " at night"};
    const char* minutes_rough_array[14] = {"",
                                           "five past ",
                                           "ten past ",
                                           "quarter past ",
                                           "twenty past ",
                                           "twenty-five past ",
                                           "half past ",
                                           "twenty-five to ",
                                           "twenty to ",
                                           "quarter to ",
                                           "ten to ",
                                           "five to "
                                           ""};
    const char* minutes_accurate_array[6] = {"", "just gone ", "a little after ", "coming up to ", "almost "};
    const char* days_array[9] = {"", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    const char* months_array[13] =
      {"", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    const char* months_numbers_array[32] = {
      "zero",          "first",        "second",       "third",          "fourth",        "fifth",        "sixth",         "seventh",
      "eighth",        "ninth",        "tenth",        "eleventh",       "twelfth",       "thirteenth",   "fourteenth",    "fifteenth",
      "sixteenth",     "seventeenth",  "eighteenth",   "nineteenth",     "twentieth",     "twenty-first", "twenty-second", "twenty-third",
      "twenty-fourth", "twenty-fifth", "twenty-sixth", "twenty-seventh", "twenty-eighth", "twenty-ninth", "thirtieth",     "thirty-first"};
    if (displayedHour != hour || displayedMinute != minute) {
      displayedHour = hour;
      displayedMinute = minute;

      if (minute > 32) {
        hour_adjusted = (hour + 1) % 24;
      } else {
        hour_adjusted = hour;
      }

      if (hour_adjusted != 0 && hour_adjusted != 12) {
        sprintf(part_day, "%s", part_day_word_array[hour_adjusted / 3]);
      } else {
        sprintf(part_day, "");
      }
      sprintf(words,
              "%s%s%s%s",
              minutes_accurate_array[(minute) % 5],
              minutes_rough_array[(minute + 2) / 5],
              hour_word_array[hour_adjusted],
              part_day);
      // Make first letter Uppercase
      words[0] = words[0] - 32;

      lv_label_set_text_fmt(label_time, "%s", words);
      lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, -40);

      if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
        lv_label_set_text_fmt(label_date,
                              "%s, %s of %s",
                              days_array[static_cast<uint8_t>(dayOfWeek)],
                              months_numbers_array[static_cast<uint8_t>(day)],
                              months_array[static_cast<uint8_t>(month)]);
  
        // Specific dates have specific names
        if (static_cast<uint8_t>(month) == 1 && static_cast<uint8_t>(day) == 1) {
          lv_label_set_text_fmt(label_date, "%s, New Year's Day", days_array[static_cast<uint8_t>(dayOfWeek)]);
        }
        if (static_cast<uint8_t>(month) == 3 && static_cast<uint8_t>(day) == 15) {
          lv_label_set_text_fmt(label_date, "%s on the Ides of March", days_array[static_cast<uint8_t>(dayOfWeek)]);
        }
        if (static_cast<uint8_t>(month) == 4 && static_cast<uint8_t>(day) == 1) {
          lv_label_set_text_fmt(label_date, "%s, ERROR C Nonsense in BASIC", days_array[static_cast<uint8_t>(dayOfWeek)]);
        }
        if (static_cast<uint8_t>(month) == 7 && static_cast<uint8_t>(day) == 1) {
          lv_label_set_text_fmt(label_date, "%s - O'Canada", days_array[static_cast<uint8_t>(dayOfWeek)]);
        }
        if (static_cast<uint8_t>(month) == 10 && static_cast<uint8_t>(day) == 31) {
          lv_label_set_text_fmt(label_date, "%s on Halloween", days_array[static_cast<uint8_t>(dayOfWeek)]);
        }
        if (static_cast<uint8_t>(month) == 12 && static_cast<uint8_t>(day) == 24) {
          lv_label_set_text_fmt(label_date, "%s, Christmas Eve", days_array[static_cast<uint8_t>(dayOfWeek)]);
        }
        if (static_cast<uint8_t>(month) == 12 && static_cast<uint8_t>(day) == 25) {
          lv_label_set_text_fmt(label_date, "%s, Christmas Day", days_array[static_cast<uint8_t>(dayOfWeek)]);
        }
        if (static_cast<uint8_t>(month) == 12 && static_cast<uint8_t>(day) == 26) {
          lv_label_set_text_fmt(label_date, "%s, Boxing Day", days_array[static_cast<uint8_t>(dayOfWeek)]);
        }
        if (static_cast<uint8_t>(month) == 12 && static_cast<uint8_t>(day) == 31) {
          lv_label_set_text_fmt(label_date, "%s, New Year's Eve", days_array[static_cast<uint8_t>(dayOfWeek)]);
        }
  
        // Maximum lenght of date in words
        //      lv_label_set_text_fmt(label_date,"Wednesday, twenty-seventh of September");
  
        lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  
        currentYear = year;
        currentMonth = month;
        currentDayOfWeek = dayOfWeek;
        currentDay = day;
      }
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepIcon);
  }
}
