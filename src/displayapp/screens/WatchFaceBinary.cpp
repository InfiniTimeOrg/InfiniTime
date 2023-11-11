#include "displayapp/screens/WatchFaceBinary.h"

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


#define LV_COLOR_MATRIX_GREEN LV_COLOR_MAKE(0x00, 0xC0, 0x16)
#define BINARY_ON_COLOR LV_COLOR_RED
#define BINARY_OFF_COLOR LV_COLOR_GRAY

const uint8_t pointSize = 32;
const uint8_t widthSpacer = 8;
const uint8_t offsetX = ( LV_HOR_RES - ( ( pointSize * 6 ) + 2 * widthSpacer ) ) / 5;
const uint8_t offsetY = pointSize * 1.25;
const int16_t hourY = 32;
const int16_t minuteY = hourY + offsetY;
const int16_t secondY = minuteY + offsetY;
const uint8_t tflHeight = (pointSize - 3) / 2;
const uint8_t tflOffsetX = 4;
const int8_t tflOffsetY = -4; 
bool is12HourModeSet;

using namespace Pinetime::Applications::Screens;

WatchFaceBinary::WatchFaceBinary(Controllers::DateTime& dateTimeController,
                                   const Controllers::Battery& batteryController,
                                   const Controllers::Ble& bleController,
                                   Controllers::NotificationManager& notificationManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController)
  : currentDateTime {{}},
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
  lv_obj_align(label_date, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_local_text_color(label_date, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  for ( uint8_t i = 0; i < 6; i++ )
  {
    // Hours
    if ( 5 > i )
    {
      hour_points[i] = lv_obj_create(lv_scr_act(), nullptr);

      lv_obj_set_style_local_bg_color(hour_points[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
      lv_obj_set_style_local_radius(hour_points[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
      lv_obj_set_size(hour_points[i], pointSize, pointSize);
      lv_obj_set_pos(hour_points[i], widthSpacer + (pointSize + offsetX) * i, hourY);
    }
    // Minutes
    minute_points[i] = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_bg_color(minute_points[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
    lv_obj_set_style_local_radius(minute_points[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_size(minute_points[i], pointSize, pointSize);
    lv_obj_set_pos(minute_points[i], widthSpacer + (pointSize + offsetX) * i, minuteY);
    // Seconds
    second_points[i] = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_bg_color(second_points[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
    lv_obj_set_style_local_radius(second_points[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_size(second_points[i], pointSize, pointSize);
    lv_obj_set_pos(second_points[i], widthSpacer + (pointSize + offsetX) * i, secondY);
  }

  label_am = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_am, "AM");
  lv_obj_set_pos(label_am, widthSpacer + tflOffsetX + (pointSize + offsetX) * 5, hourY + tflOffsetY);
  lv_obj_set_style_local_text_color(label_am, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
  lv_obj_set_size(label_am, pointSize, tflHeight);
  
  label_pm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_pm, "PM");
  lv_obj_set_pos(label_pm, widthSpacer + tflOffsetX + (pointSize + offsetX) * 5, hourY + tflOffsetY + tflHeight + 1);
  lv_obj_set_style_local_text_color(label_pm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
  lv_obj_set_size(label_pm, pointSize, tflHeight);

  is12HourModeSet = ( settingsController.GetClockType() == Controllers::Settings::ClockType::H12 );
  lv_obj_set_hidden(hour_points[4], is12HourModeSet);
  lv_obj_set_hidden(label_am, !is12HourModeSet);
  lv_obj_set_hidden(label_pm, !is12HourModeSet);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceBinary::~WatchFaceBinary() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceBinary::Refresh() {
  statusIcons.Update();

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    uint8_t seconds = dateTimeController.Seconds();

    if ( settingsController.GetClockType() == Controllers::Settings::ClockType::H12 )
    {
      if ( !is12HourModeSet )
      {
        is12HourModeSet = true;
      }

      if ( ( 12 <= hour ) && ( 24 > hour ) )
      {
        if ( 12 != hour )
        {
          hour -= 12;
        }
        lv_obj_set_style_local_text_color(label_am, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
        lv_obj_set_style_local_text_color(label_pm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BINARY_ON_COLOR);
      }
      else
      {
        if ( 0 == hour )
        {
          hour = 12;
        }
        lv_obj_set_style_local_text_color(label_am, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BINARY_ON_COLOR);
        lv_obj_set_style_local_text_color(label_pm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
      }
      hour <<= 1;
    }
    else if ( is12HourModeSet )
    {
      is12HourModeSet = false;
    }
    lv_obj_set_hidden(hour_points[4], is12HourModeSet);
    lv_obj_set_hidden(label_am, !is12HourModeSet);
    lv_obj_set_hidden(label_pm, !is12HourModeSet);

    for ( uint8_t i = 0; i < 6; i++ )
    {
      // Hours
      if ( 5 > i )
      {
        switch ( hour >> i & 0b1 )
        {
          case 1:
            lv_obj_set_style_local_bg_color(hour_points[4 - i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, BINARY_ON_COLOR);
          break;
          default:
            lv_obj_set_style_local_bg_color(hour_points[4 - i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
          break;
        }
      }
      // Minutes
      switch ( minute >> i & 0b1 )
      {
        case 1:
          lv_obj_set_style_local_bg_color(minute_points[5 - i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, BINARY_ON_COLOR);
        break;
        default:
          lv_obj_set_style_local_bg_color(minute_points[5 - i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
        break;
      }
      // Seconds
      switch ( seconds >> i & 0b1 )
      {
        case 1:
          lv_obj_set_style_local_bg_color(second_points[5 - i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, BINARY_ON_COLOR);
        break;
        default:
          lv_obj_set_style_local_bg_color(second_points[5 - i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, BINARY_OFF_COLOR);
        break;
      }
    }

    currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      uint8_t day = dateTimeController.Day();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(label_date,
                              "%s %d %s %d",
                              dateTimeController.DayOfWeekShortToString(),
                              day,
                              dateTimeController.MonthShortToString(),
                              year);
      } else {
        lv_label_set_text_fmt(label_date,
                              "%s %s %d %d",
                              dateTimeController.DayOfWeekShortToString(),
                              dateTimeController.MonthShortToString(),
                              day,
                              year);
      }
      lv_obj_realign(label_date);
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
}
