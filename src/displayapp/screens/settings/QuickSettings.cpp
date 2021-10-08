#include "QuickSettings.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/BatteryIcon.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void ButtonEventHandler(lv_event_t *event) {
    auto* screen = static_cast<QuickSettings*>(lv_event_get_user_data(event));
    screen->OnButtonEvent(lv_event_get_target(event), event);
  }

  static void lv_update_task(struct _lv_timer_t* task) {
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

  // This is the distance (padding) between all objects on this screen.
  static constexpr uint8_t innerDistance = 10;

  // Time
  label_time = lv_label_create(lv_scr_act());
  lv_label_set_text_fmt(label_time, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_obj_set_style_text_align(label_time, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(label_time, LV_ALIGN_TOP_LEFT, 0, 0);

  batteryIcon = lv_label_create(lv_scr_act());
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
  lv_obj_align(batteryIcon, LV_ALIGN_TOP_RIGHT, 0, 0);

  static constexpr uint8_t barHeight = 20 + innerDistance;
  static constexpr uint8_t buttonHeight = (LV_VER_RES_MAX - barHeight - innerDistance) / 2;
  static constexpr uint8_t buttonWidth = (LV_HOR_RES_MAX - innerDistance) / 2; // wide buttons
  //static constexpr uint8_t buttonWidth = buttonHeight; // square buttons
  static constexpr uint8_t buttonXOffset = (LV_HOR_RES_MAX - buttonWidth * 2 - innerDistance) / 2;

  lv_style_init(&btn_style);
  lv_style_set_radius(&btn_style, buttonHeight / 4);
  lv_style_set_bg_color(&btn_style, lv_color_hex(0x111111));

  btn1 = lv_btn_create(lv_scr_act());
  btn1->user_data = this;
  lv_obj_add_event_cb(btn1, ButtonEventHandler, LV_EVENT_ALL, btn1->user_data);
  lv_obj_add_style(btn1, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(btn1, buttonWidth, buttonHeight);
  lv_obj_align(btn1, LV_ALIGN_TOP_LEFT, buttonXOffset, barHeight);

  btn1_lvl = lv_label_create(btn1);
  lv_obj_set_style_text_font(btn1_lvl, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(btn1_lvl, brightness.GetIcon());

  btn2 = lv_btn_create(lv_scr_act());
  btn2->user_data = this;
  lv_obj_add_event_cb(btn2, ButtonEventHandler, LV_EVENT_ALL, btn2->user_data);
  lv_obj_add_style(btn2, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(btn2, buttonWidth, buttonHeight);
  lv_obj_align(btn2, LV_ALIGN_TOP_RIGHT, - buttonXOffset, barHeight);

  lv_obj_t* lbl_btn;
  lbl_btn = lv_label_create(btn2);
  lv_obj_set_style_text_font(lbl_btn, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(lbl_btn, Symbols::highlight);

  btn3 = lv_btn_create(lv_scr_act());
  btn3->user_data = this;
  lv_obj_add_event_cb(btn3, ButtonEventHandler, LV_EVENT_ALL, btn3->user_data);
  lv_obj_add_flag(btn3, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_style(btn3, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(btn3, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_size(btn3, buttonWidth, buttonHeight);
  lv_obj_align(btn3, LV_ALIGN_BOTTOM_LEFT, buttonXOffset, 0);

  btn3_lvl = lv_label_create(btn3);
  lv_obj_set_style_text_font(btn3_lvl, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);

  if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::ON) {
    lv_obj_add_state(btn3, LV_STATE_CHECKED);
    lv_label_set_text_static(btn3_lvl, Symbols::notificationsOn);
  } else {
    lv_label_set_text_static(btn3_lvl, Symbols::notificationsOff);
  }

  btn4 = lv_btn_create(lv_scr_act());
  btn4->user_data = this;
  lv_obj_add_event_cb(btn4, ButtonEventHandler, LV_EVENT_ALL, btn4->user_data);
  lv_obj_add_style(btn4, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(btn4, buttonWidth, buttonHeight);
  lv_obj_align(btn4, LV_ALIGN_BOTTOM_RIGHT, - buttonXOffset, 0);

  lbl_btn = lv_label_create(btn4);
  lv_obj_set_style_text_font(lbl_btn, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(lbl_btn, Symbols::settings);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  taskUpdate = lv_timer_create(lv_update_task, 5000, this);
}

QuickSettings::~QuickSettings() {
  lv_style_reset(&btn_style);
  lv_timer_del(taskUpdate);
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void QuickSettings::UpdateScreen() {
  lv_label_set_text_fmt(label_time, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
}

void QuickSettings::OnButtonEvent(lv_obj_t* object, lv_event_t* event) {
  if (object == btn2 && lv_event_get_code(event) == LV_EVENT_CLICKED) {

    running = false;
    app->StartApp(Apps::FlashLight, DisplayApp::FullRefreshDirections::None);

  } else if (object == btn1 && lv_event_get_code(event) == LV_EVENT_CLICKED) {

    brightness.Step();
    lv_label_set_text_static(btn1_lvl, brightness.GetIcon());
    settingsController.SetBrightness(brightness.Level());

  } else if (object == btn3 && lv_event_get_code(event) == LV_EVENT_VALUE_CHANGED) {

    if (lv_obj_get_state(btn3) & LV_STATE_CHECKED) {
      settingsController.SetNotificationStatus(Controllers::Settings::Notification::ON);
      motorController.RunForDuration(35);
      lv_label_set_text_static(btn3_lvl, Symbols::notificationsOn);
    } else {
      settingsController.SetNotificationStatus(Controllers::Settings::Notification::OFF);
      lv_label_set_text_static(btn3_lvl, Symbols::notificationsOff);
    }

  } else if (object == btn4 && lv_event_get_code(event) == LV_EVENT_CLICKED) {
    running = false;
    settingsController.SetSettingsMenu(0);
    app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
  }
}
