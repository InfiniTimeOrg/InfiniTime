#include "displayapp/screens/WatchFaceInfineat.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/BleIcon.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFaceInfineat*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

WatchFaceInfineat::WatchFaceInfineat(Controllers::DateTime& dateTimeController,
                                     const Controllers::Battery& batteryController,
                                     const Controllers::Ble& bleController,
                                     Controllers::NotificationManager& notificationManager,
                                     Controllers::Settings& settingsController,
                                     Controllers::MotionController& motionController,
                                     Controllers::FS& filesystem)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController} {
  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/teko.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_teko = lv_font_load("F:/fonts/teko.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/bebas.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_bebas = lv_font_load("F:/fonts/bebas.bin");
  }

  // Side Cover
  static constexpr lv_point_t linePoints[nLines][2] = {{{30, 25}, {68, -8}},
                                                       {{26, 167}, {43, 216}},
                                                       {{27, 40}, {27, 196}},
                                                       {{12, 182}, {65, 249}},
                                                       {{17, 99}, {17, 144}},
                                                       {{14, 81}, {40, 127}},
                                                       {{14, 163}, {40, 118}},
                                                       {{-20, 124}, {25, -11}},
                                                       {{-29, 89}, {27, 254}}};

  static constexpr lv_style_int_t lineWidths[nLines] = {18, 15, 14, 22, 20, 18, 18, 52, 48};

  for (int i = 0; i < nLines; i++) {
    lines[i] = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_line_width(lines[i], LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lineWidths[i]);
    lv_obj_set_style_local_line_color(lines[i],
                                      LV_LINE_PART_MAIN,
                                      LV_STATE_DEFAULT,
                                      lv_color_hex(infineatColors.orange[settingsController.GetInfineatColorIndex() * nLines + i]));
    lv_line_set_points(lines[i], linePoints[i], 2);
  }

  logoPine = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(logoPine, "F:/images/pine_small.bin");
  lv_obj_set_pos(logoPine, 15, 106);

  lineBattery = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_line_width(lineBattery, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 24);
  lv_obj_set_style_local_line_color(lineBattery,
                                    LV_LINE_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    lv_color_hex(infineatColors.orange[settingsController.GetInfineatColorIndex() * nLines + 4]));
  lv_obj_set_style_local_line_opa(lineBattery, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 190);
  lineBatteryPoints[0] = {27, 105};
  lineBatteryPoints[1] = {27, 106};
  lv_line_set_points(lineBattery, lineBatteryPoints, 2);
  lv_obj_move_foreground(lineBattery);

  notificationIcon = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(notificationIcon,
                                  LV_BTN_PART_MAIN,
                                  LV_STATE_DEFAULT,
                                  lv_color_hex(infineatColors.orange[settingsController.GetInfineatColorIndex() * nLines + 7]));
  lv_obj_set_style_local_radius(notificationIcon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_size(notificationIcon, 13, 13);
  lv_obj_set_hidden(notificationIcon, true);

  if (!settingsController.GetInfineatShowSideCover()) {
    ToggleBatteryIndicatorColor(false);
    for (auto& line : lines) {
      lv_obj_set_hidden(line, true);
    }
  }

  timeContainer = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(timeContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_size(timeContainer, 185, 185);
  lv_obj_align(timeContainer, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);

  labelHour = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelHour, "01");
  lv_obj_set_style_local_text_font(labelHour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_bebas);
  lv_obj_align(labelHour, timeContainer, LV_ALIGN_IN_TOP_MID, 0, 0);

  labelMinutes = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelMinutes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_bebas);
  lv_label_set_text_static(labelMinutes, "00");
  lv_obj_align(labelMinutes, timeContainer, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  labelTimeAmPm = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelTimeAmPm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);

  lv_label_set_text_static(labelTimeAmPm, "");
  lv_obj_align(labelTimeAmPm, timeContainer, LV_ALIGN_OUT_RIGHT_TOP, 0, 15);

  dateContainer = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(dateContainer, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_size(dateContainer, 60, 30);
  lv_obj_align(dateContainer, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 5);

  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_set_style_local_text_font(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
  lv_obj_align(labelDate, dateContainer, LV_ALIGN_IN_TOP_MID, 0, 0);
  lv_label_set_text_static(labelDate, "Mon 01");

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, dateContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_set_style_local_text_font(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 10, 0);
  lv_label_set_text_static(stepValue, "0");

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  // Setting buttons
  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblClose = lv_label_create(btnClose, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  btnNextColor = lv_btn_create(lv_scr_act(), nullptr);
  btnNextColor->user_data = this;
  lv_obj_set_size(btnNextColor, 60, 60);
  lv_obj_align(btnNextColor, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 0);
  lv_obj_set_style_local_bg_opa(btnNextColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblNextColor = lv_label_create(btnNextColor, nullptr);
  lv_label_set_text_static(lblNextColor, ">");
  lv_obj_set_event_cb(btnNextColor, event_handler);
  lv_obj_set_hidden(btnNextColor, true);

  btnPrevColor = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevColor->user_data = this;
  lv_obj_set_size(btnPrevColor, 60, 60);
  lv_obj_align(btnPrevColor, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 0);
  lv_obj_set_style_local_bg_opa(btnPrevColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblPrevColor = lv_label_create(btnPrevColor, nullptr);
  lv_label_set_text_static(lblPrevColor, "<");
  lv_obj_set_event_cb(btnPrevColor, event_handler);
  lv_obj_set_hidden(btnPrevColor, true);

  btnToggleCover = lv_btn_create(lv_scr_act(), nullptr);
  btnToggleCover->user_data = this;
  lv_obj_set_size(btnToggleCover, 60, 60);
  lv_obj_align(btnToggleCover, lv_scr_act(), LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_local_bg_opa(btnToggleCover, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  const char* labelToggle = settingsController.GetInfineatShowSideCover() ? "ON" : "OFF";
  lblToggle = lv_label_create(btnToggleCover, nullptr);
  lv_label_set_text_static(lblToggle, labelToggle);
  lv_obj_set_event_cb(btnToggleCover, event_handler);
  lv_obj_set_hidden(btnToggleCover, true);

  // Button to access the settings
  btnSettings = lv_btn_create(lv_scr_act(), nullptr);
  btnSettings->user_data = this;
  lv_obj_set_size(btnSettings, 150, 150);
  lv_obj_align(btnSettings, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_radius(btnSettings, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 30);
  lv_obj_set_style_local_bg_opa(btnSettings, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_set_event_cb(btnSettings, event_handler);
  labelBtnSettings = lv_label_create(btnSettings, nullptr);
  lv_obj_set_style_local_text_font(labelBtnSettings, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(labelBtnSettings, Symbols::settings);
  lv_obj_set_hidden(btnSettings, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceInfineat::~WatchFaceInfineat() {
  lv_task_del(taskRefresh);

  if (font_bebas != nullptr) {
    lv_font_free(font_bebas);
  }
  if (font_teko != nullptr) {
    lv_font_free(font_teko);
  }

  lv_obj_clean(lv_scr_act());
}

bool WatchFaceInfineat::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_get_hidden(btnSettings)) {
    lv_obj_set_hidden(btnSettings, false);
    savedTick = lv_tick_get();
    return true;
  }
  // Prevent screen from sleeping when double tapping with settings on
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && !lv_obj_get_hidden(btnClose)) {
    return true;
  }
  return false;
}

void WatchFaceInfineat::CloseMenu() {
  settingsController.SaveSettings();
  lv_obj_set_hidden(btnClose, true);
  lv_obj_set_hidden(btnNextColor, true);
  lv_obj_set_hidden(btnPrevColor, true);
  lv_obj_set_hidden(btnToggleCover, true);
}

bool WatchFaceInfineat::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnClose)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFaceInfineat::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    bool showSideCover = settingsController.GetInfineatShowSideCover();
    int colorIndex = settingsController.GetInfineatColorIndex();

    if (object == btnSettings) {
      lv_obj_set_hidden(btnSettings, true);
      lv_obj_set_hidden(btnClose, false);
      lv_obj_set_hidden(btnNextColor, !showSideCover);
      lv_obj_set_hidden(btnPrevColor, !showSideCover);
      lv_obj_set_hidden(btnToggleCover, false);
    }
    if (object == btnClose) {
      CloseMenu();
    }
    if (object == btnToggleCover) {
      settingsController.SetInfineatShowSideCover(!showSideCover);
      ToggleBatteryIndicatorColor(!showSideCover);
      for (auto& line : lines) {
        lv_obj_set_hidden(line, showSideCover);
      }
      lv_obj_set_hidden(btnNextColor, showSideCover);
      lv_obj_set_hidden(btnPrevColor, showSideCover);
      const char* labelToggle = showSideCover ? "OFF" : "ON";
      lv_label_set_text_static(lblToggle, labelToggle);
    }
    if (object == btnNextColor) {
      colorIndex = (colorIndex + 1) % nColors;
      settingsController.SetInfineatColorIndex(colorIndex);
    }
    if (object == btnPrevColor) {
      colorIndex -= 1;
      if (colorIndex < 0)
        colorIndex = nColors - 1;
      settingsController.SetInfineatColorIndex(colorIndex);
    }
    if (object == btnNextColor || object == btnPrevColor) {
      for (int i = 0; i < nLines; i++) {
        lv_obj_set_style_local_line_color(lines[i],
                                          LV_LINE_PART_MAIN,
                                          LV_STATE_DEFAULT,
                                          lv_color_hex(infineatColors.orange[colorIndex * nLines + i]));
      }
      lv_obj_set_style_local_line_color(lineBattery,
                                        LV_LINE_PART_MAIN,
                                        LV_STATE_DEFAULT,
                                        lv_color_hex(infineatColors.orange[colorIndex * nLines + 4]));
      lv_obj_set_style_local_bg_color(notificationIcon,
                                      LV_BTN_PART_MAIN,
                                      LV_STATE_DEFAULT,
                                      lv_color_hex(infineatColors.orange[colorIndex * nLines + 7]));
    }
  }
}

void WatchFaceInfineat::Refresh() {
  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_obj_set_hidden(notificationIcon, !notificationState.Get());
    lv_obj_align(notificationIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);
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

    int hour = time.hours().count();
    auto minute = time.minutes().count();

    char minutesChar[3];
    sprintf(minutesChar, "%02d", static_cast<int>(minute));

    char hoursChar[3];
    char ampmChar[3];

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      if (hour < 12) {
        if (hour == 0) {
          hour = 12;
        }
        sprintf(ampmChar, "AM");
      } else { // hour >= 12
        if (hour != 12) {
          hour = hour - 12;
        }
        sprintf(ampmChar, "PM");
      }
    }
    sprintf(hoursChar, "%02d", hour);

    if ((hoursChar[0] != displayedChar[0]) || (hoursChar[1] != displayedChar[1]) || (minutesChar[0] != displayedChar[2]) ||
        (minutesChar[1] != displayedChar[3])) {
      displayedChar[0] = hoursChar[0];
      displayedChar[1] = hoursChar[1];
      displayedChar[2] = minutesChar[0];
      displayedChar[3] = minutesChar[1];

      lv_label_set_text_fmt(labelHour, "%s", hoursChar);
      lv_label_set_text_fmt(labelMinutes, "%s", minutesChar);
    }

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      lv_label_set_text(labelTimeAmPm, ampmChar);
      lv_obj_align(labelTimeAmPm, timeContainer, LV_ALIGN_OUT_RIGHT_TOP, 0, 10);
      lv_obj_align(labelHour, timeContainer, LV_ALIGN_IN_TOP_MID, 0, 5);
      lv_obj_align(labelMinutes, timeContainer, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_fmt(labelDate, "%s %02d", dateTimeController.DayOfWeekShortToStringLow(), day);
      lv_obj_realign(labelDate);

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  isCharging = batteryController.IsCharging();
  if (batteryController.IsCharging()) { // Charging battery animation
    chargingBatteryPercent += 1;
    if (chargingBatteryPercent > 100) {
      chargingBatteryPercent = batteryPercentRemaining.Get();
    }
    SetBatteryLevel(chargingBatteryPercent);
  } else if (isCharging.IsUpdated() || batteryPercentRemaining.IsUpdated()) {
    chargingBatteryPercent = batteryPercentRemaining.Get();
    SetBatteryLevel(chargingBatteryPercent);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated()) {
    lv_label_set_text_static(bleIcon, BleIcon::GetIcon(bleState.Get()));
    lv_obj_align(bleIcon, dateContainer, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 10, 0);
    lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  }

  if (!lv_obj_get_hidden(btnSettings)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      lv_obj_set_hidden(btnSettings, true);
      savedTick = 0;
    }
  }
}

void WatchFaceInfineat::SetBatteryLevel(uint8_t batteryPercent) {
  // starting point (y) + Pine64 logo height * (100 - batteryPercent) / 100
  lineBatteryPoints[1] = {27, static_cast<lv_coord_t>(105 + 32 * (100 - batteryPercent) / 100)};
  lv_line_set_points(lineBattery, lineBatteryPoints, 2);
}

void WatchFaceInfineat::ToggleBatteryIndicatorColor(bool showSideCover) {
  if (!showSideCover) { // make indicator and notification icon color white
    lv_obj_set_style_local_image_recolor_opa(logoPine, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
    lv_obj_set_style_local_image_recolor(logoPine, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_style_local_line_color(lineBattery, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_color(notificationIcon, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  } else {
    lv_obj_set_style_local_image_recolor_opa(logoPine, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_line_color(lineBattery,
                                      LV_LINE_PART_MAIN,
                                      LV_STATE_DEFAULT,
                                      lv_color_hex(infineatColors.orange[settingsController.GetInfineatColorIndex() * nLines + 4]));
    lv_obj_set_style_local_bg_color(notificationIcon,
                                    LV_BTN_PART_MAIN,
                                    LV_STATE_DEFAULT,
                                    lv_color_hex(infineatColors.orange[settingsController.GetInfineatColorIndex() * nLines + 7]));
  }
}

bool WatchFaceInfineat::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/teko.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/bebas.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/images/pine_small.bin", LFS_O_RDONLY) < 0) {
    return false;
  }

  filesystem.FileClose(&file);
  return true;
}
