#include "displayapp/screens/WatchFacePhotoStyle.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/Colors.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
#include "displayapp/DisplayApp.h"
#include "components/ble/SimpleWeatherService.h"


using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<WatchFacePhotoStyle*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

WatchFacePhotoStyle::WatchFacePhotoStyle(Controllers::DateTime& dateTimeController,
                                   const Controllers::Battery& batteryController,
                                   const Controllers::Ble& bleController,
                                   Controllers::NotificationManager& notificationManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController,
                                   Controllers::SimpleWeatherService& weatherService,
                                   Controllers::FS& filesystem)
  : currentDateTime {{}},
    dateTimeController {dateTimeController},
    notificationManager {notificationManager},
    bleController {bleController},
    batteryController {batteryController},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    weatherService {weatherService} {

  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/images/watch_photo.bin", LFS_O_RDONLY) >= 0) {
    isAvailable = true;
    filesystem.FileClose(&file);
  }

  backgroundColor = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_color(backgroundColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorBG()));
  lv_obj_set_style_local_radius(backgroundColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(backgroundColor, 240, 240);
  lv_obj_align(backgroundColor, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_hidden(backgroundColor, (settingsController.GetPhotoFaceShowPhoto() && isAvailable));

  watchPhoto = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(watchPhoto, "F:/images/watch_photo.bin");
  lv_obj_align(watchPhoto, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_hidden(watchPhoto, (!settingsController.GetPhotoFaceShowPhoto() || !isAvailable));

  statusIcons = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(statusIcons, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text_static(statusIcons, "");
  lv_obj_align(statusIcons, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  temperatureTop = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(temperatureTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text(temperatureTop, "");
  lv_obj_align(temperatureTop, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  weatherIconBottom = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(weatherIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_obj_set_style_local_text_color(weatherIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text(weatherIconBottom, "");
  lv_obj_align(weatherIconBottom, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 64);
  lv_obj_set_auto_realign(weatherIconBottom, true);

  temperatureBottom = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(temperatureBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text(temperatureBottom, "");
  lv_obj_align(temperatureBottom, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -32, 64);

  labelDateTop = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelDateTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_obj_align(labelDateTop, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  labelDateBottom = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelDateBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_obj_align(labelDateBottom, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 64);

  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_obj_set_style_local_text_font(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -30, 60);

  batteryValueTop = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text_static(batteryValueTop, "0% battery");
  lv_obj_align(batteryValueTop, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  batteryValueBottom = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text_static(batteryValueBottom, "0%");
  lv_obj_align(batteryValueBottom, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 64);

  batteryIconBottom = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(batteryIconBottom, Symbols::plug);
  lv_obj_set_style_local_text_color(batteryIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_obj_align(batteryIconBottom, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 64);

  heartbeatValueTop = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text_static(heartbeatValueTop, "--- BPM");
  lv_obj_align(heartbeatValueTop, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 0);

  heartbeatIconBottom = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIconBottom, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_obj_align(heartbeatIconBottom, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 64);

  heartbeatValueBottom = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text_static(heartbeatValueBottom, "---");
  lv_obj_align(heartbeatValueBottom, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -35, 64);

  stepValueTop = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text(stepValueTop, "0 steps");
  lv_obj_align(stepValueTop, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 0);

  stepIconBottom = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text_static(stepIconBottom, Symbols::shoe);
  lv_obj_align(stepIconBottom, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -5, 64);
  lv_obj_set_auto_realign(stepIconBottom, true);

  stepValueBottom = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(settingsController.GetPhotoFaceColorTime()));
  lv_label_set_text_static(stepValueBottom, "0");
  lv_obj_align(stepValueBottom, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -35, 64);

  btnNextTime = lv_btn_create(lv_scr_act(), nullptr);
  btnNextTime->user_data = this;
  lv_obj_set_size(btnNextTime, 60, 60);
  lv_obj_align(btnNextTime, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, -80);
  lv_obj_set_style_local_bg_opa(btnNextTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblNextTime = lv_label_create(btnNextTime, nullptr);
  lv_label_set_text_static(lblNextTime, ">");
  lv_obj_set_event_cb(btnNextTime, event_handler);
  lv_obj_set_hidden(btnNextTime, true);

  btnPrevTime = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevTime->user_data = this;
  lv_obj_set_size(btnPrevTime, 60, 60);
  lv_obj_align(btnPrevTime, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, -80);
  lv_obj_set_style_local_bg_opa(btnPrevTime, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblPrevTime = lv_label_create(btnPrevTime, nullptr);
  lv_label_set_text_static(lblPrevTime, "<");
  lv_obj_set_event_cb(btnPrevTime, event_handler);
  lv_obj_set_hidden(btnPrevTime, true);

  btnNextBG = lv_btn_create(lv_scr_act(), nullptr);
  btnNextBG->user_data = this;
  lv_obj_set_size(btnNextBG, 60, 60);
  lv_obj_align(btnNextBG, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -15, 0);
  lv_obj_set_style_local_bg_opa(btnNextBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblNextBG = lv_label_create(btnNextBG, nullptr);
  lv_label_set_text_static(lblNextBG, ">");
  lv_obj_set_event_cb(btnNextBG, event_handler);
  lv_obj_set_hidden(btnNextBG, true);

  btnPrevBG = lv_btn_create(lv_scr_act(), nullptr);
  btnPrevBG->user_data = this;
  lv_obj_set_size(btnPrevBG, 60, 60);
  lv_obj_align(btnPrevBG, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 15, 0);
  lv_obj_set_style_local_bg_opa(btnPrevBG, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblPrevBG = lv_label_create(btnPrevBG, nullptr);
  lv_label_set_text_static(lblPrevBG, "<");
  lv_obj_set_event_cb(btnPrevBG, event_handler);
  lv_obj_set_hidden(btnPrevBG, true);

  btnReset = lv_btn_create(lv_scr_act(), nullptr);
  btnReset->user_data = this;
  lv_obj_set_size(btnReset, 60, 60);
  lv_obj_align(btnReset, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(btnReset, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblReset = lv_label_create(btnReset, nullptr);
  lv_label_set_text_static(lblReset, "Rst");
  lv_obj_set_event_cb(btnReset, event_handler);
  lv_obj_set_hidden(btnReset, true);

  btnPhotoToggle = lv_btn_create(lv_scr_act(), nullptr);
  btnPhotoToggle->user_data = this;
  lv_obj_set_size(btnPhotoToggle, 210, 60);
  lv_obj_align(btnPhotoToggle, lv_scr_act(), LV_ALIGN_CENTER, 0, 80);
  lv_obj_set_style_local_bg_opa(btnPhotoToggle, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  const char* labelToggle = "Use Photo";
  lblToggle = lv_label_create(btnPhotoToggle, nullptr);
  lv_label_set_text_static(lblToggle, labelToggle);
  lv_obj_set_event_cb(btnPhotoToggle, event_handler);
  lv_obj_set_hidden(btnPhotoToggle, true);

  btnClose = lv_btn_create(lv_scr_act(), nullptr);
  btnClose->user_data = this;
  lv_obj_set_size(btnClose, 60, 60);
  lv_obj_align(btnClose, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);
  lv_obj_set_style_local_bg_opa(btnClose, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblClose = lv_label_create(btnClose, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnClose, event_handler);
  lv_obj_set_hidden(btnClose, true);

  btnTopContent = lv_btn_create(lv_scr_act(), nullptr);
  btnTopContent->user_data = this;
  lv_obj_set_size(btnTopContent, 160, 60);
  lv_obj_align(btnTopContent, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);
  lv_obj_set_style_local_bg_opa(btnTopContent, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblTopContent = lv_label_create(btnTopContent, nullptr);
  lv_label_set_text_static(lblTopContent, "Top");
  lv_obj_set_event_cb(btnTopContent, event_handler);
  lv_obj_set_hidden(btnTopContent, true);

  btnBottomContent = lv_btn_create(lv_scr_act(), nullptr);
  btnBottomContent->user_data = this;
  lv_obj_set_size(btnBottomContent, 160, 60);
  lv_obj_align(btnBottomContent, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);
  lv_obj_set_style_local_bg_opa(btnBottomContent, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblBottomContent = lv_label_create(btnBottomContent, nullptr);
  lv_label_set_text_static(lblBottomContent, "Bottom");
  lv_obj_set_event_cb(btnBottomContent, event_handler);
  lv_obj_set_hidden(btnBottomContent, true);

  btnSetColor = lv_btn_create(lv_scr_act(), nullptr);
  btnSetColor->user_data = this;
  lv_obj_set_size(btnSetColor, 150, 60);
  lv_obj_align(btnSetColor, lv_scr_act(), LV_ALIGN_CENTER, 0, -40);
  lv_obj_set_style_local_radius(btnSetColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_opa(btnSetColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_event_cb(btnSetColor, event_handler);
  lv_obj_t* lblSetColor = lv_label_create(btnSetColor, nullptr);
  lv_obj_set_style_local_text_font(lblSetColor, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(lblSetColor, Symbols::paintbrushLg);
  lv_obj_set_hidden(btnSetColor, true);

  btnSetOpts = lv_btn_create(lv_scr_act(), nullptr);
  btnSetOpts->user_data = this;
  lv_obj_set_size(btnSetOpts, 150, 60);
  lv_obj_align(btnSetOpts, lv_scr_act(), LV_ALIGN_CENTER, 0, 40);
  lv_obj_set_style_local_radius(btnSetOpts, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_opa(btnSetOpts, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_event_cb(btnSetOpts, event_handler);
  lv_obj_t* lblSetOpts = lv_label_create(btnSetOpts, nullptr);
  lv_obj_set_style_local_text_font(lblSetOpts, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(lblSetOpts, Symbols::settings);
  lv_obj_set_hidden(btnSetOpts, true);

  btnNoPhoto = lv_btn_create(lv_scr_act(), nullptr);
  btnNoPhoto->user_data = this;
  lv_obj_set_size(btnNoPhoto, 150, 150);
  lv_obj_align(btnNoPhoto, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_radius(btnNoPhoto, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_opa(btnNoPhoto, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_t* lblNoPhoto = lv_label_create(btnNoPhoto, nullptr);
  lv_label_set_text_static(lblNoPhoto, "No Photo\nFound");
  lv_obj_set_event_cb(btnNoPhoto, event_handler);
  lv_obj_set_hidden(btnNoPhoto, true);

  RefreshContent();
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFacePhotoStyle::~WatchFacePhotoStyle() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

bool WatchFacePhotoStyle::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::LongTap) && lv_obj_get_hidden(btnClose)) {
    lv_obj_set_hidden(btnSetOpts, false);
    lv_obj_set_hidden(btnSetColor, false);
    savedTick = lv_tick_get();
    return true;
  }
  if ((event == Pinetime::Applications::TouchEvents::DoubleTap) && !lv_obj_get_hidden(btnClose)) {
    return true;
  }
  return false;
}

void WatchFacePhotoStyle::CloseMenu() {
  if (!lv_obj_get_hidden(btnNoPhoto)) {
    lv_obj_set_hidden(btnNoPhoto, true);
    lv_obj_set_hidden(btnClose, false);
    lv_obj_set_hidden(btnReset, false);
    lv_obj_set_hidden(btnPhotoToggle, false);
    lv_obj_set_hidden(btnNextBG, false);
    lv_obj_set_hidden(btnPrevBG, false);
    lv_obj_set_hidden(btnNextTime, false);
    lv_obj_set_hidden(btnPrevTime, false);
  } else {
    settingsController.SaveSettings();
    lv_obj_set_hidden(btnSetOpts, true);
    lv_obj_set_hidden(btnSetColor, true);
    lv_obj_set_hidden(btnClose, true);
    lv_obj_set_hidden(btnReset, true);
    lv_obj_set_hidden(btnPhotoToggle, true);
    lv_obj_set_hidden(btnNextBG, true);
    lv_obj_set_hidden(btnTopContent, true);
    lv_obj_set_hidden(btnBottomContent, true);
    lv_obj_set_hidden(btnNextTime, true);
    lv_obj_set_hidden(btnPrevBG, true);
    lv_obj_set_hidden(btnPrevTime, true);
  }
}

bool WatchFacePhotoStyle::OnButtonPushed() {
  if (!lv_obj_get_hidden(btnClose) || !lv_obj_get_hidden(btnNoPhoto)) {
    CloseMenu();
    return true;
  }
  return false;
}

void WatchFacePhotoStyle::RefreshContent() {
  auto contentTop = settingsController.GetPhotoFaceContentTop();
  auto contentBottom = settingsController.GetPhotoFaceContentBottom();

  lv_obj_set_hidden(labelDateTop, contentTop != Controllers::Settings::ContentStyle::Date);
  lv_obj_set_hidden(stepValueTop, contentTop != Controllers::Settings::ContentStyle::Steps);
  lv_obj_set_hidden(batteryValueTop, contentTop != Controllers::Settings::ContentStyle::Battery);
  lv_obj_set_hidden(heartbeatValueTop, contentTop != Controllers::Settings::ContentStyle::Heart);
  lv_obj_set_hidden(temperatureTop, contentTop != Controllers::Settings::ContentStyle::Weather);
  
  lv_obj_set_hidden(labelDateBottom, contentBottom != Controllers::Settings::ContentStyle::Date);
  lv_obj_set_hidden(stepIconBottom, contentBottom != Controllers::Settings::ContentStyle::Steps);
  lv_obj_set_hidden(stepValueBottom, contentBottom != Controllers::Settings::ContentStyle::Steps);
  lv_obj_set_hidden(batteryValueBottom, contentBottom != Controllers::Settings::ContentStyle::Battery);
  lv_obj_set_hidden(batteryIconBottom, contentBottom != Controllers::Settings::ContentStyle::Battery);
  lv_obj_set_hidden(heartbeatIconBottom, contentBottom != Controllers::Settings::ContentStyle::Heart);
  lv_obj_set_hidden(heartbeatValueBottom, contentBottom != Controllers::Settings::ContentStyle::Heart);
  lv_obj_set_hidden(weatherIconBottom, contentBottom != Controllers::Settings::ContentStyle::Weather);
  lv_obj_set_hidden(temperatureBottom, contentBottom != Controllers::Settings::ContentStyle::Weather);
}

void WatchFacePhotoStyle::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated() || powerPresent.IsUpdated()) {
    lv_label_set_text_fmt(batteryValueTop, "%d%%", batteryPercentRemaining.Get());
    lv_label_set_text_fmt(batteryValueBottom, "%d%%", batteryPercentRemaining.Get());
    if (batteryController.IsPowerPresent()) {
      lv_label_ins_text(batteryValueTop, LV_LABEL_POS_LAST, " charging");
      lv_label_ins_text(batteryValueBottom, LV_LABEL_POS_LAST, "  ");
      lv_label_set_text(batteryIconBottom, Symbols::plug);
    } else {
      lv_label_ins_text(batteryValueTop, LV_LABEL_POS_LAST, " battery");
      lv_label_set_text(batteryIconBottom, "");
    }
    lv_obj_realign(batteryValueTop);
    lv_obj_realign(batteryValueBottom);
    lv_obj_realign(weatherIconBottom);
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (bleState.IsUpdated() or notificationState.IsUpdated()) {
    lv_label_set_text_fmt(statusIcons, "");
    if (notificationState.Get()) {
      lv_label_ins_text(statusIcons, LV_LABEL_POS_LAST, Symbols::bell);
    }
    lv_label_ins_text(statusIcons, LV_LABEL_POS_LAST, BleIcon::GetIcon(bleState.Get()));

    lv_obj_align(statusIcons, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  }

  currentDateTime = std::chrono::time_point_cast<std::chrono::minutes>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();

    if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
      if (hour == 0) {
        hour = 12;
      } else if (hour > 12) {
        hour = hour - 12;
      }
      lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
    } else {
      lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
    }
    lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 26);

    currentDate = std::chrono::time_point_cast<days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint8_t day = dateTimeController.Day();
      lv_label_set_text_fmt(labelDateTop,
                            "%s %d",
                            dateTimeController.DayOfWeekShortToString(),
                            day);
      lv_label_set_text_fmt(labelDateBottom, 
                            "%s %d", 
                            dateTimeController.DayOfWeekShortToString(),
                            day);
      lv_obj_realign(labelDateTop);
      lv_obj_realign(labelDateBottom);
    }
  }

  currentWeather = weatherService.Current();
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature;
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = Controllers::SimpleWeatherService::CelsiusToFahrenheit(temp);
      }
      temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
      lv_label_set_text_fmt(temperatureTop, "%s %i°", Symbols::GetSimpleCondition(optCurrentWeather->iconId), temp);
      lv_label_set_text_fmt(temperatureBottom, "%d°", temp);
      lv_label_set_text(weatherIconBottom, Symbols::GetSymbol(optCurrentWeather->iconId));
    } else {
      lv_label_set_text_static(temperatureTop, "No Data ---°");
      lv_label_set_text_static(temperatureBottom, "");
      lv_label_set_text(weatherIconBottom, Symbols::ban);
    }
    lv_obj_realign(temperatureTop);
    lv_obj_realign(temperatureBottom);
    lv_obj_realign(weatherIconBottom);
  }

  if (!lv_obj_get_hidden(btnSetOpts) || !lv_obj_get_hidden(btnNoPhoto)) {
    if ((savedTick > 0) && (lv_tick_get() - savedTick > 3000)) {
      CloseMenu();
      savedTick = 0;
    }
  }

  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_label_set_text_fmt(heartbeatValueBottom, "%d", heartbeat.Get());
      lv_label_set_text_fmt(heartbeatValueTop, "%d BPM", heartbeat.Get());
    } else {
      lv_label_set_text_static(heartbeatValueBottom, "---");
      lv_label_set_text_static(heartbeatValueTop, "--- BPM");
    }

    lv_obj_realign(heartbeatIconBottom);
    lv_obj_realign(heartbeatValueBottom);
    lv_obj_realign(heartbeatValueTop);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValueTop, "%lu steps", stepCount.Get());
    lv_label_set_text_fmt(stepValueBottom, "%lu", stepCount.Get());
    lv_obj_realign(stepValueTop);
    lv_obj_realign(stepValueBottom);
    lv_obj_realign(stepIconBottom);
  }
}

void WatchFacePhotoStyle::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  auto valueTime = settingsController.GetPhotoFaceColorTime();
  auto valueBG = settingsController.GetPhotoFaceColorBG();
  auto showPhoto = settingsController.GetPhotoFaceShowPhoto();
  auto contentValueTop = settingsController.GetPhotoFaceContentTop();
  auto contentValueBottom = settingsController.GetPhotoFaceContentBottom();

  if (event == LV_EVENT_CLICKED) {
    if (object == btnNextTime) {
      valueTime = GetNext(valueTime);
      if (valueTime == valueBG && !showPhoto) {
        valueTime = GetNext(valueTime);
      }
      settingsController.SetPhotoFaceColorTime(valueTime);
      lv_obj_set_style_local_text_color(statusIcons, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(temperatureTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(weatherIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(temperatureBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(labelDateTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(labelDateBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(stepIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(stepValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(stepValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(heartbeatValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(heartbeatIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(heartbeatValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(batteryValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(batteryValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(batteryIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
    }
    if (object == btnPrevTime) {
      valueTime = GetPrevious(valueTime);
      if (valueTime == valueBG && !showPhoto) {
        valueTime = GetPrevious(valueTime);
      }
      settingsController.SetPhotoFaceColorTime(valueTime);
      lv_obj_set_style_local_text_color(statusIcons, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(temperatureTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(weatherIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(temperatureBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(labelDateTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(labelDateBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(stepIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(stepValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(stepValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(heartbeatValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(heartbeatIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(heartbeatValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(batteryValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(batteryValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
      lv_obj_set_style_local_text_color(batteryIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(valueTime));
    }
    if (object == btnNextBG) {
      valueBG = GetNext(valueBG);
      if (valueBG == valueTime) {
        valueBG = GetNext(valueBG);
      }
      settingsController.SetPhotoFaceColorBG(valueBG);
      lv_obj_set_style_local_bg_color(backgroundColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBG));
    }
    if (object == btnPrevBG) {
      valueBG = GetPrevious(valueBG);
      if (valueBG == valueTime) {
        valueBG = GetPrevious(valueBG);
      }
      settingsController.SetPhotoFaceColorBG(valueBG);
      lv_obj_set_style_local_bg_color(backgroundColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBG));
    }
    if (object == btnReset) {
      settingsController.SetPhotoFaceShowPhoto(false);
      settingsController.SetPhotoFaceColorTime(Controllers::Settings::Colors::White);
      lv_obj_set_style_local_text_color(statusIcons, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(temperatureTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(weatherIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(temperatureBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(labelDateTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(labelDateBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(label_time, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(stepIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(stepValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(stepValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(heartbeatValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(heartbeatIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(heartbeatValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(batteryValueTop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(batteryValueBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      lv_obj_set_style_local_text_color(batteryIconBottom, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::White));
      settingsController.SetPhotoFaceColorBG(Controllers::Settings::Colors::Black);
      lv_obj_set_style_local_bg_color(backgroundColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(Controllers::Settings::Colors::Black));
      lv_obj_set_hidden(btnNextBG, false);
      lv_obj_set_hidden(btnPrevBG, false);
      const char* labelToggle = "Use Photo";
      lv_label_set_text_static(lblToggle, labelToggle);
    }
    if (object == btnPhotoToggle) {
      if (!isAvailable) {
        lv_obj_set_hidden(btnClose, true);
        lv_obj_set_hidden(btnReset, true);
        lv_obj_set_hidden(btnPhotoToggle, true);
        lv_obj_set_hidden(btnNextBG, true);
        lv_obj_set_hidden(btnPrevBG, true);
        lv_obj_set_hidden(btnNextTime, true);
        lv_obj_set_hidden(btnPrevTime, true);
        lv_obj_set_hidden(btnNoPhoto, false);
        savedTick = lv_tick_get();
        return;
      }
      settingsController.SetPhotoFaceShowPhoto(!showPhoto);
      if (showPhoto) {
        if (valueBG == valueTime) {
          valueBG = GetNext(valueBG);
        }
        lv_obj_set_style_local_bg_color(backgroundColor, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Convert(valueBG));
        lv_obj_set_hidden(backgroundColor, false);
        lv_obj_set_hidden(btnNextBG, false);
        lv_obj_set_hidden(btnPrevBG, false);
        lv_obj_set_hidden(watchPhoto, true);
      } else {
        lv_obj_set_hidden(backgroundColor, true);
        lv_obj_set_hidden(btnNextBG, true);
        lv_obj_set_hidden(btnPrevBG, true);
        lv_obj_set_hidden(watchPhoto, false);
      }
      lv_obj_set_hidden(watchPhoto, showPhoto);
      const char* labelToggle = !showPhoto ? "Use Color" : "Use Photo";
      lv_label_set_text_static(lblToggle, labelToggle);
    }
    if (object == btnNoPhoto) {
      CloseMenu();
    }
    if (object == btnTopContent) {
      contentValueTop = GetNextContent(contentValueTop);
      settingsController.SetPhotoFaceContentTop(contentValueTop);
      RefreshContent();
    }
    if (object == btnBottomContent) {
      contentValueBottom = GetNextContent(contentValueBottom);
      settingsController.SetPhotoFaceContentBottom(contentValueBottom);
      RefreshContent();
    }
    if (object == btnSetColor) {
      lv_obj_set_hidden(btnSetOpts, true);
      lv_obj_set_hidden(btnSetColor, true);
      lv_obj_set_hidden(btnClose, false);
      lv_obj_set_hidden(btnReset, false);
      lv_obj_set_hidden(btnPhotoToggle, false);
      if (!showPhoto || !isAvailable) {
          lv_obj_set_hidden(btnNextBG, false);
          lv_obj_set_hidden(btnPrevBG, false);
      }
      lv_obj_set_hidden(btnNextTime, false);
      lv_obj_set_hidden(btnPrevTime, false);
      const char* labelToggle = (!showPhoto || !isAvailable) ? "Use Photo" : "Use Color";
      lv_label_set_text_static(lblToggle, labelToggle);
    }
    if (object == btnSetOpts) {
      lv_obj_set_hidden(btnSetOpts, true);
      lv_obj_set_hidden(btnSetColor, true);
      lv_obj_set_hidden(btnClose, false);
      lv_obj_set_hidden(btnTopContent, false);
      lv_obj_set_hidden(btnBottomContent, false);
    }
    if (object == btnClose) {
      CloseMenu();
    }
  }
}

Pinetime::Controllers::Settings::ContentStyle WatchFacePhotoStyle::GetNextContent(Pinetime::Controllers::Settings::ContentStyle content) {
  auto contentAsInt = static_cast<uint8_t>(content);
  Pinetime::Controllers::Settings::ContentStyle nextContent;
  if (contentAsInt < 5) {
    nextContent = static_cast<Controllers::Settings::ContentStyle>(contentAsInt + 1);
  } else {
    nextContent = static_cast<Controllers::Settings::ContentStyle>(0);
  }
  return nextContent;
}

Pinetime::Controllers::Settings::Colors WatchFacePhotoStyle::GetNext(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors nextColor;
  if (colorAsInt < 17) {
    nextColor = static_cast<Controllers::Settings::Colors>(colorAsInt + 1);
  } else {
    nextColor = static_cast<Controllers::Settings::Colors>(0);
  }
  return nextColor;
}

Pinetime::Controllers::Settings::Colors WatchFacePhotoStyle::GetPrevious(Pinetime::Controllers::Settings::Colors color) {
  auto colorAsInt = static_cast<uint8_t>(color);
  Pinetime::Controllers::Settings::Colors prevColor;

  if (colorAsInt > 0) {
    prevColor = static_cast<Controllers::Settings::Colors>(colorAsInt - 1);
  } else {
    prevColor = static_cast<Controllers::Settings::Colors>(17);
  }
  return prevColor;
}