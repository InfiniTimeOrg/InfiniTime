#include "displayapp/screens/WatchFaceHorizon.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "components/motion/MotionController.h"

using namespace Pinetime::Applications::Screens;

WatchFaceHorizon::WatchFaceHorizon(DisplayApp* app,
                                   Controllers::DateTime& dateTimeController,
                                   Controllers::Battery& batteryController,
                                   Controllers::Settings& settingsController,
                                   Controllers::MotionController& motionController,
                                   Controllers::FS& filesystem)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    settingsController {settingsController},
    motionController {motionController} {
  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/pinecone_28.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_pinecone_28 = lv_font_load("F:/fonts/pinecone_28.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/pinecone_70.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_pinecone_70 = lv_font_load("F:/fonts/pinecone_70.bin");
  }

  // Black background covering the whole screen
  background = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(background, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_size(background, 240, 240);
  lv_obj_align(background, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  // Battery indicator line
  lineBatteryBg = lv_line_create(lv_scr_act(), nullptr);
  lineBatteryFg = lv_line_create(lv_scr_act(), nullptr);

  lv_style_init(&lineBatteryBgStyle);
  lv_style_set_line_width(&lineBatteryBgStyle, LV_STATE_DEFAULT, 4);
  lv_style_set_line_color(&lineBatteryBgStyle, LV_STATE_DEFAULT, lv_color_hex(0x181818));
  lv_obj_add_style(lineBatteryBg, LV_LINE_PART_MAIN, &lineBatteryBgStyle);
  lineBatteryBgPoints[0] = {116, 40};
  lineBatteryBgPoints[1] = {116, 200};
  lv_line_set_points(lineBatteryBg, lineBatteryBgPoints, 2);

  lv_style_init(&lineBatteryFgStyle);
  lv_style_set_line_width(&lineBatteryFgStyle, LV_STATE_DEFAULT, 4);
  lv_style_set_line_color(&lineBatteryFgStyle, LV_STATE_DEFAULT, lv_color_hex(hourlyColors[0]));
  lv_obj_add_style(lineBatteryFg, LV_LINE_PART_MAIN, &lineBatteryFgStyle);
  lineBatteryFgPoints[0] = {116, 40};
  lineBatteryFgPoints[1] = {116, 200};
  lv_line_set_points(lineBatteryFg, lineBatteryFgPoints, 2);

  // Hour indicator lines at bottom
  for (int i = 0; i < 24; i++) {
    hourLines[i] = lv_line_create(lv_scr_act(), nullptr);
    lv_style_init(&hourLineStyles[i]);
    lv_style_set_line_width(&hourLineStyles[i], LV_STATE_DEFAULT, 5);
    lv_style_set_line_color(&hourLineStyles[i], LV_STATE_DEFAULT, lv_color_hex(hourlyColors[i]));
    lv_obj_add_style(hourLines[i], LV_LINE_PART_MAIN, &hourLineStyles[i]);
    hourLinePoints[i][0] = {static_cast<lv_coord_t>(i * 10), 237};
    hourLinePoints[i][1] = {static_cast<lv_coord_t>((i + 1) * 10), 237};
    lv_line_set_points(hourLines[i], hourLinePoints[i], 2);
  }

  // Hour (split digits due to non-monospaced font)
  labelHourFirstDigit = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelHourFirstDigit, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_pinecone_70);
  lv_label_set_text(labelHourFirstDigit, "0");
  lv_obj_align(labelHourFirstDigit, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 130, -43);

  labelHourSecondDigit = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelHourSecondDigit, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_pinecone_70);
  lv_label_set_text(labelHourSecondDigit, "0");
  lv_obj_align(labelHourSecondDigit, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 174, -43);

  // Minutes (split digits due to non-monospaced font)
  labelMinutesFirstDigit = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelMinutesFirstDigit, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_pinecone_70);
  lv_label_set_text(labelMinutesFirstDigit, "0");
  lv_obj_align(labelMinutesFirstDigit, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 130, 44);

  labelMinutesSecondDigit = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelMinutesSecondDigit, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_pinecone_70);
  lv_label_set_text(labelMinutesSecondDigit, "0");
  lv_obj_align(labelMinutesSecondDigit, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 174, 44);

  // Day of week
  labelDayOfWeek = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelDayOfWeek, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xbebebe));
  lv_obj_set_style_local_text_font(labelDayOfWeek, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_pinecone_28);
  lv_obj_align(labelDayOfWeek, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -140, -60);
  lv_label_set_text(labelDayOfWeek, "MON");

  // Month
  labelMonth = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelMonth, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_pinecone_28);
  lv_obj_align(labelMonth, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -140, -20);
  lv_label_set_text(labelMonth, "JAN");

  // Day of month
  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xbebebe));
  lv_obj_set_style_local_text_font(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_pinecone_28);
  lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -140, 20);
  lv_label_set_text(labelDate, "01");

  // Number of steps
  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_pinecone_28);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -140, 60);
  lv_label_set_text(stepValue, "0");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceHorizon::~WatchFaceHorizon() {
  lv_task_del(taskRefresh);

  // Reset styles
  lv_style_reset(&lineBatteryFgStyle);
  lv_style_reset(&lineBatteryBgStyle);

  for (int i = 0; i < 24; i++) {
    lv_style_reset(&hourLineStyles[i]);
  }

  // Free font resources
  if (font_pinecone_28 != nullptr) {
    lv_font_free(font_pinecone_28);
  }

  if (font_pinecone_70 != nullptr) {
    lv_font_free(font_pinecone_70);
  }

  lv_obj_clean(lv_scr_act());
}

void WatchFaceHorizon::Refresh() {
  currentDateTime = dateTimeController.CurrentDateTime();

  if (currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime - dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto month = static_cast<Pinetime::Controllers::DateTime::Months>(static_cast<unsigned>(yearMonthDay.month()));
    auto day = static_cast<unsigned>(yearMonthDay.day());
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    int64_t hour = time.hours().count();
    int64_t minute = time.minutes().count();

    char minutesChar[3];
    sprintf(minutesChar, "%02d", static_cast<int>(minute));

    char hoursChar[3];
    int displayHour = hour;

    // Account for 12-hour time
    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      if (hour < 12) {
        if (hour == 0) {
          displayHour = 12;
        }
      } else {
        if (hour != 12) {
          displayHour = hour - 12;
        }
      }
    }
    sprintf(hoursChar, "%02d", displayHour);

    // Hour has updated
    if (hoursChar[0] != displayedChar[0] || hoursChar[1] != displayedChar[1]) {
      displayedChar[0] = hoursChar[0];
      displayedChar[1] = hoursChar[1];

      // Update colors appropriately
      lv_obj_set_style_local_text_color(labelHourFirstDigit, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(hourlyColors[hour]));
      lv_obj_set_style_local_text_color(labelHourSecondDigit, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(hourlyColors[hour]));
      lv_obj_set_style_local_text_color(labelMonth, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(hourlyColors[hour]));
      lv_style_set_line_color(&lineBatteryFgStyle, LV_STATE_DEFAULT, lv_color_hex(hourlyColors[hour]));

      lv_label_set_text_fmt(labelHourFirstDigit, "%c", hoursChar[0]);
      lv_label_set_text_fmt(labelHourSecondDigit, "%c", hoursChar[1]);

      // Update hour bar on bottom
      for (int i = 0; i < 24; i++) {
        if (i <= hour) {
          lv_obj_set_hidden(hourLines[i], false);
        } else {
          lv_obj_set_hidden(hourLines[i], true);
        }
      }
    }

    // Minutes have updated
    if (minutesChar[0] != displayedChar[2] || minutesChar[1] != displayedChar[3]) {
      displayedChar[2] = minutesChar[0];
      displayedChar[3] = minutesChar[1];

      lv_label_set_text_fmt(labelMinutesFirstDigit, "%c", minutesChar[0]);
      lv_label_set_text_fmt(labelMinutesSecondDigit, "%c", minutesChar[1]);
    }

    // Date has updated
    if ((month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_fmt(labelDayOfWeek, "%s", dateTimeController.DayOfWeekShortToString());
      lv_label_set_text_fmt(labelMonth, "%s", dateTimeController.MonthShortToString());
      lv_label_set_text_fmt(labelDate, "%d", day);

      lv_obj_realign(labelDayOfWeek);
      lv_obj_realign(labelMonth);
      lv_obj_realign(labelDate);

      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  // Set battery line
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    lineBatteryFgPoints[0] = {116, static_cast<lv_coord_t>(200 - (1.6 * batteryPercentRemaining.Get()))};
    lv_line_set_points(lineBatteryFg, lineBatteryFgPoints, 2);
  }

  // Set step count
  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
  }
}

bool WatchFaceHorizon::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/pinecone_28.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/pinecone_70.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  return true;
}
