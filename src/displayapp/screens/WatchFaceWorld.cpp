#include "displayapp/screens/WatchFaceWorld.h"

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

WatchFaceWorld::WatchFaceWorld(DisplayApp* app,
                               Controllers::DateTime& dateTimeController,
                               Controllers::Battery& batteryController,
                               Controllers::Ble& bleController,
                               Controllers::NotificationManager& notificationManager,
                               Controllers::Settings& settingsController,
                               Controllers::HeartRateController& heartRateController,
                               Controllers::MotionController& motionController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
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
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 58);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 10);

  for (size_t i = 0; i < sizeof(label_worldtime) / sizeof(*label_worldtime); i++) {
    label_worldtime[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_align(label_worldtime[i], lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -5, 66 - i * 30);
    lv_label_set_text_static(label_worldtime[i], "");
    lv_obj_set_style_local_text_color(label_worldtime[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xBBBBBB));

    label_worlddescription[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_align(label_worlddescription[i], lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 5, 66 - i * 30);
    lv_label_set_text_static(label_worlddescription[i], "");
    lv_obj_set_style_local_text_color(label_worlddescription[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xBBBBBB));
  }

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -30, -55);

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

WatchFaceWorld::~WatchFaceWorld() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceWorld::Refresh() {
  statusIcons.Update();

  notificationState = notificationManager.AreNewNotificationsAvailable();
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

    bool minuteUpdated = false;

    if (displayedHour != hour || displayedMinute != minute) {
      minuteUpdated = true;
      displayedHour = hour;
      displayedMinute = minute;

      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
      lv_obj_realign(label_time);
    }

    uint8_t nrWorldClocks = 0;
    bool worldClocksChanged = false;

    for (size_t i = 0; i < sizeof(label_worldtime) / sizeof(*label_worldtime); i++) {
      // count enabled clocks
      // needs to be done before next check to be accurate
      if (dateTimeController.isWorldTimeEnabled(i)) {
        nrWorldClocks++;
      }

      worldClockOffsets[i] = dateTimeController.worldOffset(i);
      // only redraw if something changed
      if (!(worldClockOffsets[i].IsUpdated()) and !minuteUpdated) {
        continue;
      }
      // resetDirty
      worldClockOffsets[i].Get();

      worldClocksChanged = true;

      if (dateTimeController.isWorldTimeEnabled(i)) {
        auto worldDateTime = dateTimeController.WorldDateTime(i);
        auto worldDp = date::floor<date::days>(worldDateTime);
        auto worldTime = date::make_time(worldDateTime - worldDp);
        uint8_t worldHour = worldTime.hours().count();
        uint8_t worldMinute = worldTime.minutes().count();

        lv_label_set_text_fmt(label_worldtime[i], "%02d:%02d", worldHour, worldMinute);
        lv_obj_realign(label_worldtime[i]);

        lv_label_set_text_fmt(label_worlddescription[i], dateTimeController.worldDescription(i), hour, minute);
        lv_obj_realign(label_worlddescription[i]);
      } else {
        lv_label_set_text_static(label_worldtime[i], "");
        lv_obj_realign(label_worldtime[i]);

        lv_label_set_text_static(label_worlddescription[i], "");
        lv_obj_realign(label_worlddescription[i]);
      }
    }

    // realign time and date if worldClocks changed
    if (worldClocksChanged) {
      if (nrWorldClocks < 3) {
        lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);
        lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 168 - nrWorldClocks * 30);
      } else {
        lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
        lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 158 - nrWorldClocks * 26);
      }
      lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 74 - nrWorldClocks * 17);
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_fmt(label_date,
                            "%s %d %s %d",
                            dateTimeController.DayOfWeekShortToString(),
                            day,
                            dateTimeController.MonthShortToString(),
                            year);

      lv_obj_realign(label_date);

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
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
