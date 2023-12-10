#include "displayapp/screens/WatchFaceFuturistic.h"
#include <cmath>
#include <lvgl/lvgl.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/NotificationIcon.h"
#include "components/heartrate/HeartRateController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // Based on the tango terminal colour theme.
  constexpr lv_color_t innerTextColor = LV_COLOR_MAKE(0xee, 0xee, 0xec);
  constexpr lv_color_t hoursArcColor = LV_COLOR_MAKE(0xef, 0x29, 0x29);
  constexpr lv_color_t minutesArcColor = LV_COLOR_MAKE(0x72, 0x9f, 0xcf);
  constexpr lv_color_t secondsArcColor = LV_COLOR_MAKE(0x8a, 0xe2, 0x34);
  constexpr lv_color_t outerTextColor = LV_COLOR_MAKE(0xfc, 0xe9, 0x4f);
}

WatchFaceFuturistic::WatchFaceFuturistic(Pinetime::Applications::DisplayApp* app,
                                         Controllers::DateTime& dateTimeController,
                                         Controllers::Battery& batteryController,
                                         Controllers::Ble& bleController,
                                         Controllers::NotificationManager& notificationManager,
                                         Controllers::HeartRateController& heartRateController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    heartRateController {heartRateController} {

  // heart beat icon
  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, outerTextColor);
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  // heart rate
  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(heartbeatValue, "000");
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, outerTextColor);
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  // plug icon
  plugIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(plugIcon, Symbols::plug);
  lv_obj_set_style_local_text_color(plugIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, outerTextColor);
  lv_obj_align(plugIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  // battery percentage (only displayed when charging)
  batteryStatus = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryStatus, "000%");
  lv_obj_set_style_local_text_color(batteryStatus, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, outerTextColor);
  lv_obj_align(batteryStatus, plugIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  // low battery alert message
  batteryAlert = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryAlert, "low bat");
  lv_obj_set_style_local_text_color(batteryAlert, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, outerTextColor);
  lv_obj_align(batteryAlert, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  arcSec = lv_arc_create(lv_scr_act(), nullptr);
  lv_obj_set_size(arcSec, 220, 220);
  lv_arc_set_range(arcSec, 0, 60);
  lv_obj_align(arcSec, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_arc_set_rotation(arcSec, 270);
  lv_arc_set_bg_angles(arcSec, 0, 360);
  lv_arc_set_adjustable(arcSec, false);
  lv_arc_set_start_angle(arcSec, 0);
  lv_obj_set_style_local_line_color(arcSec, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, secondsArcColor);
  lv_obj_set_style_local_line_width(arcSec, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 8);
  lv_obj_set_style_local_line_rounded(arcSec, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, false);
  lv_obj_set_style_local_line_width(arcSec, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);

  arcMin = lv_arc_create(lv_scr_act(), nullptr);
  lv_obj_set_size(arcMin, 180, 180);
  lv_arc_set_range(arcMin, 0, 60);
  lv_obj_align(arcMin, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_arc_set_rotation(arcMin, 270);
  lv_arc_set_bg_angles(arcMin, 0, 360);
  lv_arc_set_adjustable(arcMin, false);
  lv_arc_set_start_angle(arcMin, 0);
  lv_obj_set_style_local_line_color(arcMin, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, minutesArcColor);
  lv_obj_set_style_local_line_width(arcMin, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 8);
  lv_obj_set_style_local_line_rounded(arcMin, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, false);
  lv_obj_set_style_local_line_width(arcMin, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);

  arcHour = lv_arc_create(lv_scr_act(), nullptr);
  lv_obj_set_size(arcHour, 140, 140);
  lv_arc_set_range(arcHour, 0, 12);
  lv_obj_align(arcHour, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_arc_set_rotation(arcHour, 270);
  lv_arc_set_bg_angles(arcHour, 0, 360);
  lv_arc_set_adjustable(arcHour, false);
  lv_arc_set_start_angle(arcHour, 0);
  lv_obj_set_style_local_line_color(arcHour, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, hoursArcColor);
  lv_obj_set_style_local_line_width(arcHour, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 8);
  lv_obj_set_style_local_line_rounded(arcHour, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, false);
  lv_obj_set_style_local_line_width(arcHour, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);

  timeDateLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(timeDateLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, innerTextColor);
  lv_label_set_text_fmt(timeDateLabel, "[init]");
  lv_label_set_align(timeDateLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(timeDateLabel, nullptr, LV_ALIGN_CENTER, 0, -12);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  Refresh();
}

WatchFaceFuturistic::~WatchFaceFuturistic() {
  lv_task_del(taskRefresh);

  lv_obj_clean(lv_scr_act());
}

void WatchFaceFuturistic::Refresh() {
  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_hidden(heartbeatIcon, false);
      lv_obj_set_hidden(heartbeatValue, false);
      lv_label_set_text_fmt(heartbeatValue, "%03d", heartbeat.Get());
    } else {
      lv_obj_set_hidden(heartbeatIcon, true);
      lv_obj_set_hidden(heartbeatValue, true);
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  isCharging = batteryController.IsCharging();
  if (isCharging.IsUpdated() || batteryPercentRemaining.IsUpdated()) {
    if (isCharging.Get()) {
      lv_obj_set_hidden(plugIcon, false);
      lv_obj_set_hidden(batteryStatus, false);
      lv_label_set_text_fmt(batteryStatus, "%03d%%", batteryPercentRemaining.Get());
      lv_obj_set_hidden(batteryAlert, true);
    } else {
      lv_obj_set_hidden(plugIcon, true);
      lv_obj_set_hidden(batteryStatus, true);

      if (batteryPercentRemaining.Get() < 15) {
        lv_obj_set_hidden(batteryAlert, false);
      } else {
        lv_obj_set_hidden(batteryAlert, true);
      }
    }
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    uint8_t day = dateTimeController.Day();
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    uint8_t second = dateTimeController.Seconds();

    if (sSecond != second) {
      lv_arc_set_value(arcSec, second);
      sSecond = second;
    }

    if (sMinute != minute) {
      lv_label_set_text_fmt(timeDateLabel, "%02i:%02i\n%s %02i", hour, minute, dateTimeController.DayOfWeekShortToString(), day);
      lv_arc_set_value(arcMin, minute);
      sMinute = minute;
    }

    if (sHour != hour) {
      lv_arc_set_value(arcHour, (hour > 12) ? hour - 12 : hour);
      sHour = hour;
    }
  }
}
