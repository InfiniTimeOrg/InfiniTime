#include "QuickSettings.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/BatteryIcon.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void ButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<QuickSettings*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }

  static void lv_update_task(struct _lv_task_t* task) {
    auto* user_data = static_cast<QuickSettings*>(task->user_data);
    user_data->UpdateScreen();
  }
}

QuickSettings::QuickSettings(Pinetime::Applications::DisplayApp* app,
                             Pinetime::Controllers::Battery& batteryController,
                             Controllers::DateTime& dateTimeController,
                             Controllers::BrightnessController& brightness,
                             Controllers::MotorController& motorController,
                             Pinetime::Controllers::Settings& settingsController)
  : Screen(app),
    batteryController {batteryController},
    dateTimeController {dateTimeController},
    brightness {brightness},
    motorController {motorController},
    settingsController {settingsController} {

  // Time
  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(label_time, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 15, 4);

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
  lv_obj_align(batteryIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, -15, 4);

  lv_obj_t* lbl_btn;

  btn1 = lv_btn_create(lv_scr_act(), nullptr);
  btn1->user_data = this;
  lv_obj_set_event_cb(btn1, ButtonEventHandler);
  lv_obj_align(btn1, nullptr, LV_ALIGN_CENTER, -50, -30);
  lv_obj_set_style_local_radius(btn1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(btn1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
  lv_obj_set_style_local_bg_grad_dir(btn1, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);

  lv_btn_set_fit2(btn1, LV_FIT_TIGHT, LV_FIT_TIGHT);

  btn1_lvl = lv_label_create(btn1, nullptr);
  lv_obj_set_style_local_text_font(btn1_lvl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(btn1_lvl, brightness.GetIcon());

  btn2 = lv_btn_create(lv_scr_act(), nullptr);
  btn2->user_data = this;
  lv_obj_set_event_cb(btn2, ButtonEventHandler);
  lv_obj_align(btn2, nullptr, LV_ALIGN_CENTER, 50, -30);
  lv_obj_set_style_local_radius(btn2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(btn2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
  lv_obj_set_style_local_bg_grad_dir(btn2, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
  lv_btn_set_fit2(btn2, LV_FIT_TIGHT, LV_FIT_TIGHT);

  lbl_btn = lv_label_create(btn2, nullptr);
  lv_obj_set_style_local_text_font(lbl_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(lbl_btn, Symbols::highlight);

  btn3 = lv_btn_create(lv_scr_act(), nullptr);
  btn3->user_data = this;
  lv_obj_set_event_cb(btn3, ButtonEventHandler);
  lv_obj_align(btn3, nullptr, LV_ALIGN_CENTER, -50, 60);
  lv_btn_set_checkable(btn3, true);
  lv_obj_set_style_local_radius(btn3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(btn3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
  lv_obj_set_style_local_bg_grad_dir(btn3, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
  lv_obj_set_style_local_bg_color(btn3, LV_BTN_PART_MAIN, LV_STATE_CHECKED, LV_COLOR_GREEN);
  lv_obj_set_style_local_bg_grad_dir(btn1, LV_BTN_PART_MAIN, LV_STATE_CHECKED, LV_GRAD_DIR_NONE);
  lv_btn_set_fit2(btn3, LV_FIT_TIGHT, LV_FIT_TIGHT);

  btn3_lvl = lv_label_create(btn3, nullptr);
  lv_obj_set_style_local_text_font(btn3_lvl, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);

  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::ON) {
    lv_obj_add_state(btn3, LV_STATE_CHECKED);
    lv_label_set_text_static(btn3_lvl, Symbols::notificationsOn);
  } else {
    lv_label_set_text_static(btn3_lvl, Symbols::notificationsOff);
  }

  btn4 = lv_btn_create(lv_scr_act(), nullptr);
  btn4->user_data = this;
  lv_obj_set_event_cb(btn4, ButtonEventHandler);
  lv_obj_align(btn4, nullptr, LV_ALIGN_CENTER, 50, 60);
  lv_obj_set_style_local_radius(btn4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(btn4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
  lv_obj_set_style_local_bg_grad_dir(btn4, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_GRAD_DIR_NONE);
  lv_btn_set_fit2(btn4, LV_FIT_TIGHT, LV_FIT_TIGHT);

  lbl_btn = lv_label_create(btn4, nullptr);
  lv_obj_set_style_local_text_font(lbl_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_sys_48);
  lv_label_set_text_static(lbl_btn, Symbols::settings);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  taskUpdate = lv_task_create(lv_update_task, 5000, LV_TASK_PRIO_MID, this);
}

QuickSettings::~QuickSettings() {
  lv_task_del(taskUpdate);
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void QuickSettings::UpdateScreen() {
  lv_label_set_text_fmt(label_time, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
}

void QuickSettings::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (object == btn2 && event == LV_EVENT_PRESSED) {

    running = false;
    app->StartApp(Apps::FlashLight, DisplayApp::FullRefreshDirections::None);

  } else if (object == btn1 && event == LV_EVENT_PRESSED) {

    brightness.Step();
    lv_label_set_text_static(btn1_lvl, brightness.GetIcon());
    settingsController.SetBrightness(brightness.Level());

  } else if (object == btn3 && event == LV_EVENT_VALUE_CHANGED) {

    if (lv_obj_get_state(btn3, LV_BTN_PART_MAIN) & LV_STATE_CHECKED) {
      settingsController.SetVibrationStatus(Controllers::Settings::Vibration::ON);
      motorController.SetDuration(35);
      lv_label_set_text_static(btn3_lvl, Symbols::notificationsOn);
    } else {
      settingsController.SetVibrationStatus(Controllers::Settings::Vibration::OFF);
      lv_label_set_text_static(btn3_lvl, Symbols::notificationsOff);
    }

  } else if (object == btn4 && event == LV_EVENT_PRESSED) {
    running = false;
    settingsController.SetSettingsMenu(0);
    app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
  }
}

bool QuickSettings::Refresh() {
  return running;
}
