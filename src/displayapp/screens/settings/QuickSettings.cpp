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

  brightnessButton = lv_btn_create(lv_scr_act());
  brightnessButton->user_data = this;
  lv_obj_add_event_cb(brightnessButton, ButtonEventHandler, LV_EVENT_ALL, brightnessButton->user_data);
  lv_obj_add_style(brightnessButton, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(brightnessButton, buttonWidth, buttonHeight);
  lv_obj_align(brightnessButton, LV_ALIGN_TOP_LEFT, buttonXOffset, barHeight);

  brightnessLabel = lv_label_create(brightnessButton);
  lv_obj_set_style_text_font(brightnessLabel, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(brightnessLabel, brightness.GetIcon());
  lv_obj_center(brightnessLabel);

  flashlightButton = lv_btn_create(lv_scr_act());
  flashlightButton->user_data = this;
  lv_obj_add_event_cb(flashlightButton, ButtonEventHandler, LV_EVENT_ALL, flashlightButton->user_data);
  lv_obj_add_style(flashlightButton, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(flashlightButton, buttonWidth, buttonHeight);
  lv_obj_align(flashlightButton, LV_ALIGN_TOP_RIGHT, - buttonXOffset, barHeight);

  lv_obj_t* flashlightLabel;
  flashlightLabel = lv_label_create(flashlightButton);
  lv_obj_set_style_text_font(flashlightLabel, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(flashlightLabel, Symbols::highlight);
  lv_obj_center(flashlightLabel);

  notificationButton = lv_btn_create(lv_scr_act());
  notificationButton->user_data = this;
  lv_obj_add_event_cb(notificationButton, ButtonEventHandler, LV_EVENT_ALL, notificationButton->user_data);
  lv_obj_add_flag(notificationButton, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_style(notificationButton, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(notificationButton, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_size(notificationButton, buttonWidth, buttonHeight);
  lv_obj_align(notificationButton, LV_ALIGN_BOTTOM_LEFT, buttonXOffset, 0);

  notificationLabel = lv_label_create(notificationButton);
  lv_obj_set_style_text_font(notificationLabel, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_center(notificationLabel);

  if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::ON) {
    lv_obj_add_state(notificationButton, LV_STATE_CHECKED);
    lv_label_set_text_static(notificationLabel, Symbols::notificationsOn);
  } else {
    lv_label_set_text_static(notificationLabel, Symbols::notificationsOff);
  }

  settingsButton = lv_btn_create(lv_scr_act());
  settingsButton->user_data = this;
  lv_obj_add_event_cb(settingsButton, ButtonEventHandler, LV_EVENT_ALL, settingsButton->user_data);
  lv_obj_add_style(settingsButton, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(settingsButton, buttonWidth, buttonHeight);
  lv_obj_align(settingsButton, LV_ALIGN_BOTTOM_RIGHT, - buttonXOffset, 0);

  lv_obj_t *settingsLabel= lv_label_create(settingsButton);
  lv_obj_set_style_text_font(settingsLabel, &lv_font_sys_48, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(settingsLabel, Symbols::settings);
  lv_obj_center(settingsLabel);

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
  if (object == flashlightButton && lv_event_get_code(event) == LV_EVENT_CLICKED) {

    running = false;
    app->StartApp(Apps::FlashLight, DisplayApp::FullRefreshDirections::None);

  } else if (object == brightnessButton && lv_event_get_code(event) == LV_EVENT_CLICKED) {

    brightness.Step();
    lv_label_set_text_static(brightnessLabel, brightness.GetIcon());
    settingsController.SetBrightness(brightness.Level());

  } else if (object == notificationButton && lv_event_get_code(event) == LV_EVENT_VALUE_CHANGED) {

    if (lv_obj_get_state(notificationButton) & LV_STATE_CHECKED) {
      settingsController.SetNotificationStatus(Controllers::Settings::Notification::ON);
      motorController.RunForDuration(35);
      lv_label_set_text_static(notificationLabel, Symbols::notificationsOn);
    } else {
      settingsController.SetNotificationStatus(Controllers::Settings::Notification::OFF);
      lv_label_set_text_static(notificationLabel, Symbols::notificationsOff);
    }

  } else if (object == settingsButton && lv_event_get_code(event) == LV_EVENT_CLICKED) {
    running = false;
    settingsController.SetSettingsMenu(0);
    app->StartApp(Apps::Settings, DisplayApp::FullRefreshDirections::Up);
  }
}
