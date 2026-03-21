#include "displayapp/screens/WatchFaceRailway.h"
#include "displayapp/screens/WatchFaceDigital.h"
#include <lvgl/lvgl.h>
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t HourHandLength = 60;
  constexpr int16_t MinuteHandLength = 85;
}

WatchFaceRailway::WatchFaceRailway(AppControllers& controllers)
  : currentDateTime {{}}, digitalOverlay {nullptr}, overlayDismissTask {nullptr}, controllers {controllers} {

  sHour = 99;
  sMinute = 99;

  CreateAnalogFace();

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

void WatchFaceRailway::CreateAnalogFace() {
  // 12 hour notches
  hourNotchMeter = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(hourNotchMeter, 330, 12);
  lv_linemeter_set_angle_offset(hourNotchMeter, 165);
  lv_linemeter_set_value(hourNotchMeter, 0);
  lv_obj_set_size(hourNotchMeter, 240, 240);
  lv_obj_align(hourNotchMeter, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(hourNotchMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(hourNotchMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 15);
  lv_obj_set_style_local_scale_end_line_width(hourNotchMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 6);
  lv_obj_set_style_local_scale_end_color(hourNotchMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  // Minute hand
  minuteHandMeter = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(minuteHandMeter, 0, 2);
  lv_linemeter_set_angle_offset(minuteHandMeter, 0);
  lv_linemeter_set_value(minuteHandMeter, 0);
  lv_obj_set_size(minuteHandMeter, MinuteHandLength * 2, MinuteHandLength * 2);
  lv_obj_align(minuteHandMeter, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(minuteHandMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(minuteHandMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, MinuteHandLength);
  lv_obj_set_style_local_scale_end_line_width(minuteHandMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 6);
  lv_obj_set_style_local_scale_end_color(minuteHandMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  // Hour hand (slightly wider)
  hourHandMeter = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(hourHandMeter, 0, 2);
  lv_linemeter_set_angle_offset(hourHandMeter, 0);
  lv_linemeter_set_value(hourHandMeter, 0);
  lv_obj_set_size(hourHandMeter, HourHandLength * 2, HourHandLength * 2);
  lv_obj_align(hourHandMeter, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(hourHandMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(hourHandMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, HourHandLength);
  lv_obj_set_style_local_scale_end_line_width(hourHandMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 8);
  lv_obj_set_style_local_scale_end_color(hourHandMeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  // Center dot
  centerDot = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(centerDot, 12, 12);
  lv_obj_align(centerDot, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_color(centerDot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_radius(centerDot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_border_width(centerDot, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);

  // Force hand positions
  sHour = 99;
  sMinute = 99;
}

WatchFaceRailway::~WatchFaceRailway() {
  lv_task_del(taskRefresh);
  if (overlayDismissTask != nullptr) {
    lv_task_del(overlayDismissTask);
  }
  if (digitalOverlay) {
    delete digitalOverlay;
  } else {
    lv_obj_clean(lv_scr_act());
  }
}

void WatchFaceRailway::UpdateClock() {
  uint8_t hour = controllers.dateTimeController.Hours();
  uint8_t minute = controllers.dateTimeController.Minutes();

  if (sMinute != minute) {
    lv_linemeter_set_angle_offset(minuteHandMeter, minute * 6);
  }

  if (sHour != hour || sMinute != minute) {
    sHour = hour;
    sMinute = minute;
    lv_linemeter_set_angle_offset(hourHandMeter, hour * 30 + minute / 2);
  }
}

void WatchFaceRailway::Refresh() {
  if (digitalOverlay == nullptr) {
    currentDateTime = controllers.dateTimeController.CurrentDateTime();
    if (currentDateTime.IsUpdated()) {
      UpdateClock();
    }
  }
}

bool WatchFaceRailway::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::Tap) {
    if (digitalOverlay) {
      HideOverlay();
    } else {
      ShowOverlay();
    }
    return true;
  }
  return false;
}

void WatchFaceRailway::ShowOverlay() {
  // Clear analog face before showing digital
  lv_obj_clean(lv_scr_act());

  digitalOverlay = new WatchFaceDigital(controllers.dateTimeController,
                                        controllers.batteryController,
                                        controllers.bleController,
                                        controllers.alarmController,
                                        controllers.notificationManager,
                                        controllers.settingsController,
                                        controllers.heartRateController,
                                        controllers.motionController,
                                        *controllers.weatherController);

  if (overlayDismissTask != nullptr) {
    lv_task_del(overlayDismissTask);
  }
  overlayDismissTask = lv_task_create(DismissOverlayCallback, 5000, LV_TASK_PRIO_MID, this);
  lv_task_set_repeat_count(overlayDismissTask, 1);
}

void WatchFaceRailway::HideOverlay() {
  if (overlayDismissTask != nullptr) {
    lv_task_del(overlayDismissTask);
    overlayDismissTask = nullptr;
  }

  // Digital's destructor cleans all screen objects
  delete digitalOverlay;
  digitalOverlay = nullptr;

  // Recreate analog face
  CreateAnalogFace();
  UpdateClock();
}

void WatchFaceRailway::DismissOverlayCallback(lv_task_t* task) {
  auto* watchface = static_cast<WatchFaceRailway*>(task->user_data);
  watchface->overlayDismissTask = nullptr;
  watchface->HideOverlay();
}
