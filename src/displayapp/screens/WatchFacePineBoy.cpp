#include "displayapp/screens/WatchFacePineBoy.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

WatchFacePineBoy::WatchFacePineBoy(Controllers::DateTime& dateTimeController,
                                   const Controllers::Battery& batteryController,
                                   const Controllers::Ble& bleController,
                                   Controllers::NotificationManager& notificationManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController,
                                   Controllers::FS& filesystem)
  : batteryIcon(true),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController} {

  initFonts();

  lv_style_init(&menuLinesStyle);
  lv_style_set_line_width(&menuLinesStyle, LV_STATE_DEFAULT, 1);
  lv_style_set_line_color(&menuLinesStyle, LV_STATE_DEFAULT, secondaryColor);
  lv_style_set_line_rounded(&menuLinesStyle, LV_STATE_DEFAULT, false);

  drawBoy(filesystem);
  drawTabsMenu();
  drawIndicators();
  drawTime();

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFacePineBoy::~WatchFacePineBoy() {
  lv_task_del(taskRefresh);

  lv_style_reset(&menuLinesStyle);

  lv_obj_clean(lv_scr_act());
}

void WatchFacePineBoy::initFonts() {
  fontSmall = &jetbrains_mono_bold_20;
  fontNormal = &jetbrains_mono_bold_20;
  fontBig = &jetbrains_mono_extrabold_compressed;
}

// drawTime creates and aligns the labels for the current time and the AM/PM marker.
void WatchFacePineBoy::drawTime() {
  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, timeOffset);
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontBig);
  lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, primaryColor);

  label_time_ampm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_time_ampm, "");
  lv_obj_align(label_time_ampm, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, ampmOFfset);
  lv_obj_set_style_local_text_font(label_time_ampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontNormal);
  lv_obj_set_style_local_text_color(label_time_ampm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, primaryColor);
}

// drawIndicators creates the indicators for battery, calendar, steps and bluetooth.
void WatchFacePineBoy::drawIndicators() {
  indicators = lv_cont_create(lv_scr_act(), nullptr);
  lv_cont_set_layout(indicators, LV_LAYOUT_COLUMN_LEFT);
  lv_obj_set_style_local_pad_inner(indicators, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_pad_top(indicators, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_left(indicators, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_right(indicators, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_pad_bottom(indicators, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_bg_opa(indicators, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_cont_set_fit(indicators, LV_FIT_TIGHT);

  static lv_point_t icon_points[] = { {0, 0}, {26, 0}, {26, 26}, {0, 26}, {0, 0} };

  batteryBorder = lv_line_create(indicators, nullptr);
  lv_line_set_points(batteryBorder, icon_points, 5);
  lv_obj_add_style(batteryBorder, LV_LINE_PART_MAIN, &menuLinesStyle);
  lv_obj_align(batteryBorder, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  batteryIcon.Create(batteryBorder);
  lv_obj_align(batteryIcon.GetObject(), batteryBorder, LV_ALIGN_CENTER, 0, 0);

  calBorder = lv_line_create(indicators, nullptr);
  lv_line_set_points(calBorder, icon_points, 5);
  lv_obj_add_style(calBorder, LV_LINE_PART_MAIN, &menuLinesStyle);
  lv_obj_align(calBorder, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 30);

  calIcon = lv_label_create(calBorder, nullptr);
  lv_obj_align(calIcon, calBorder, LV_ALIGN_CENTER, 15, 0);
  lv_obj_set_style_local_text_color(calIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
  lv_label_set_text_static(calIcon, Screens::Symbols::clock);

  stepBorder = lv_line_create(indicators, nullptr);
  lv_line_set_points(stepBorder, icon_points, 5);
  lv_obj_add_style(stepBorder, LV_LINE_PART_MAIN, &menuLinesStyle);
  lv_obj_align(stepBorder, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 60);

  stepIcon = lv_label_create(stepBorder, nullptr);
  lv_obj_align(stepIcon, stepBorder, LV_ALIGN_CENTER, 12, 0);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
  lv_label_set_text_static(stepIcon, Screens::Symbols::shoe);

  btBorder = lv_line_create(indicators, nullptr);
  lv_line_set_points(btBorder, icon_points, 5);
  lv_obj_add_style(btBorder, LV_LINE_PART_MAIN, &menuLinesStyle);
  lv_obj_align(btBorder, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 90);

  btIcon = lv_label_create(btBorder, nullptr);
  lv_obj_align(btIcon, btBorder, LV_ALIGN_CENTER, 18, 0);
  lv_obj_set_style_local_text_color(btIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
  lv_label_set_text_static(btIcon, Screens::Symbols::bluetooth);

  calValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(calValue, calBorder, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
  lv_obj_set_style_local_text_font(calValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontNormal);
  lv_obj_set_style_local_text_color(calValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, primaryColor);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(stepValue, stepBorder, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
  lv_obj_set_style_local_text_font(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontNormal);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, primaryColor);
  lv_label_set_text_static(stepValue, "0");

  batteryValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(batteryValue, batteryBorder, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
  lv_obj_set_style_local_text_font(batteryValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontNormal);
  lv_obj_set_style_local_text_color(batteryValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, primaryColor);
  lv_label_set_text_static(batteryValue, "0%");

  btValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(btValue, btBorder, LV_ALIGN_OUT_RIGHT_MID, 6, 0);
  lv_obj_set_style_local_text_font(btValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontNormal);
  lv_obj_set_style_local_text_color(btValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, primaryColor);
  lv_label_set_text_static(btValue, "");

  lv_obj_align(indicators, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 115);
}

// drawBoy places the boy image and the heartbeat indicator. If the image
// /imgs/pine-boy-icon.bin is not found on the flash storage then it draws a simple
// rectangle where the boy image would've been.
void WatchFacePineBoy::drawBoy(Pinetime::Controllers::FS& filesystem) {

  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/imgs/pine-boy-icon.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&file);

    boyImage = lv_img_create(lv_scr_act(), nullptr);
    lv_img_set_src(boyImage, "F:/imgs/pine-boy-icon.bin");
  } else {
    boyImage = lv_line_create(lv_scr_act(), nullptr);
    static lv_point_t icon_points[] = {{0, 0},{42, 0},{42, 96},{0, 96},{0, 0},{42, 96}};
    lv_line_set_points(boyImage, icon_points, 6);
    lv_obj_add_style(boyImage, LV_LINE_PART_MAIN, &menuLinesStyle);
  }

  lv_obj_align(boyImage, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -30, -25);

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
  lv_obj_align(heartbeatIcon, boyImage, LV_ALIGN_OUT_LEFT_MID, -16, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontNormal);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_TOP_MID, 0, 0);

}

// drawTabsMenu creates the pseudo tabs in the top and adds the bottom border.
void WatchFacePineBoy::drawTabsMenu() {
  tabsMenu = lv_cont_create(lv_scr_act(), nullptr);
  lv_cont_set_layout(tabsMenu, LV_LAYOUT_ROW_MID);
  lv_cont_set_fit(tabsMenu, LV_FIT_TIGHT);
  lv_obj_set_auto_realign(tabsMenu, true);
  lv_obj_align_origo(tabsMenu, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_pad_inner(tabsMenu, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 7);
  lv_obj_set_style_local_bg_opa(tabsMenu, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  tabsLabelTime = lv_label_create(tabsMenu, nullptr);
  lv_label_set_text_static(tabsLabelTime, "TIME");
  lv_obj_set_style_local_text_font(tabsLabelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontSmall);
  lv_obj_set_style_local_text_color(tabsLabelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);

  tabsLabelComm = lv_label_create(tabsMenu, nullptr);
  lv_label_set_text_static(tabsLabelComm, "COMM");
  lv_obj_set_style_local_text_font(tabsLabelComm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontSmall);
  lv_obj_set_style_local_text_color(tabsLabelComm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);

  tabsLabelQuick = lv_label_create(tabsMenu, nullptr);
  lv_label_set_text_static(tabsLabelQuick, "QUICK");
  lv_obj_set_style_local_text_font(tabsLabelQuick, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontSmall);
  lv_obj_set_style_local_text_color(tabsLabelQuick, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);

  tabsLabelApps = lv_label_create(tabsMenu, nullptr);
  lv_label_set_text_static(tabsLabelApps, "APPS");
  lv_obj_set_style_local_text_font(tabsLabelApps, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, fontSmall);
  lv_obj_set_style_local_text_color(tabsLabelApps, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);

  lv_obj_align(tabsMenu, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);

  static lv_point_t line_points_left[] = { {0, 26}, {5, 26}, {5, 10}, {8, 10} };
  static lv_point_t line_points_right[] = { {56, 10}, {59, 10}, {59, 26}, {LV_HOR_RES, 26} };

  topUnderlineLeft = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(topUnderlineLeft, line_points_left, 4);
  lv_obj_add_style(topUnderlineLeft, LV_LINE_PART_MAIN, &menuLinesStyle);
  lv_obj_align(topUnderlineLeft, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  topUnderlineRight = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(topUnderlineRight, line_points_right, 4);
  lv_obj_add_style(topUnderlineRight, LV_LINE_PART_MAIN, &menuLinesStyle);
  lv_obj_align(topUnderlineRight, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  static lv_point_t line_points_bottom[] = { {0, LV_HOR_RES}, {LV_HOR_RES, LV_HOR_RES} };
  bottomLine = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(bottomLine, line_points_bottom, 2);
  lv_obj_add_style(bottomLine, LV_LINE_PART_MAIN, &menuLinesStyle);
  lv_obj_align(bottomLine, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, -1);
}

void WatchFacePineBoy::Refresh() {
  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    if (notificationState.Get()) {
      // has notif
      lv_obj_set_style_local_text_color(tabsLabelComm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, primaryColor);
    } else {
      // does not have notif
      lv_obj_set_style_local_text_color(tabsLabelComm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
    }
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      char ampmChar[3] = "AM";
      if (hour == 0) {
        hour = 12;
      } else if (hour == 12) {
        ampmChar[0] = 'P';
      } else if (hour > 12) {
        hour = hour - 12;
        ampmChar[0] = 'P';
      }
      lv_label_set_text(label_time_ampm, ampmChar);
      lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
      lv_obj_realign(label_time);
      lv_obj_realign(label_time_ampm);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
      lv_obj_realign(label_time);
    }

    currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint8_t day = dateTimeController.Day();
      lv_label_set_text_fmt(calValue,
                            "%s %d",
                            dateTimeController.DayOfWeekShortToString(),
                            day);
      lv_obj_realign(calValue);
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, secondaryColor);
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepBorder);
  }

  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    batteryIcon.SetBatteryPercentage(batteryPercent);

    if (powerPresent.Get()) {
      lv_label_set_text_fmt(batteryValue, "%d%% %s", batteryPercent, Screens::Symbols::plug);
    } else {
      lv_label_set_text_fmt(batteryValue, "%d%%", batteryPercent);
    }

    static constexpr int lowBatteryThreshold = 15;
    static constexpr int criticalBatteryThreshold = 5;
    if (batteryPercent > lowBatteryThreshold) {
      lv_obj_set_style_local_text_color(batteryValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, primaryColor);
      batteryIcon.SetColor(secondaryColor);
    } else if (batteryPercent > criticalBatteryThreshold) {
      lv_obj_set_style_local_text_color(batteryValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, attentionColor);
      batteryIcon.SetColor(attentionColor);
    } else {
      lv_obj_set_style_local_text_color(batteryValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, warningsColor);
      batteryIcon.SetColor(warningsColor);
    }

    lv_obj_realign(batteryValue);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    if (bleRadioEnabled.Get()) {
      if (bleState.Get()) {
        lv_label_set_text_static(btValue, "PAIRED");
      } else {
        lv_label_set_text_static(btValue, "ON");
      }
    } else {
      lv_label_set_text_static(btValue, "OFF");
    }
    lv_obj_realign(btValue);
  }
}
