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
  static constexpr uint8_t barHeight = 20;
  static constexpr uint8_t buttonHeight = (LV_VER_RES_MAX - barHeight - innerDistance*3) / 2;

  //=== Top bar
  // Time
  label_time = lv_label_create(lv_scr_act());
  lv_label_set_text_fmt(label_time, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_obj_set_style_text_align(label_time, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(label_time, LV_ALIGN_TOP_LEFT, 0, 0);
  // Battery
  batteryIcon = lv_label_create(lv_scr_act());
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
  lv_obj_align(batteryIcon, LV_ALIGN_TOP_RIGHT, 0, 0);
  
  //=== Buttons
  // Button Style
  lv_style_init(&btn_style);
  lv_style_set_radius(&btn_style, buttonHeight / 4);
  lv_style_set_bg_color(&btn_style, lv_color_hex(0x111111));
  lv_style_set_flex_grow(&btn_style, 1);
  lv_style_set_max_height(&btn_style, lv_pct(50)-innerDistance/2);
  lv_style_set_height(&btn_style, lv_pct(100));
  lv_style_set_text_font(&btn_style, &lv_font_sys_48);
  
  // Flex container
  container1 = lv_obj_create(lv_scr_act());
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(container1, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);
  lv_obj_set_size(container1, LV_HOR_RES, LV_VER_RES - barHeight);
  lv_obj_align(container1, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_gap(container1, innerDistance, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(container1, innerDistance, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  // Buttons
  brightnessButton = lv_btn_create(container1);
  brightnessButton->user_data = this;
  lv_obj_add_event_cb(brightnessButton, ButtonEventHandler, LV_EVENT_ALL, brightnessButton->user_data);
  lv_obj_add_style(brightnessButton, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  brightnessLabel = lv_label_create(brightnessButton);
  lv_label_set_text_static(brightnessLabel, brightness.GetIcon());
  lv_obj_center(brightnessLabel);

  
  flashlightButton = lv_btn_create(container1);
  flashlightButton->user_data = this;
  lv_obj_add_event_cb(flashlightButton, ButtonEventHandler, LV_EVENT_ALL, flashlightButton->user_data);
  lv_obj_add_style(flashlightButton, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  flashlightLabel = lv_label_create(flashlightButton);
  lv_label_set_text_static(flashlightLabel, Symbols::highlight);
  lv_obj_center(flashlightLabel);

  
  notificationButton = lv_btn_create(container1);
  notificationButton->user_data = this;
  lv_obj_add_event_cb(notificationButton, ButtonEventHandler, LV_EVENT_ALL, notificationButton->user_data);
  lv_obj_add_style(notificationButton, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(notificationButton, LV_OBJ_FLAG_CHECKABLE | LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
  lv_obj_set_style_bg_color(notificationButton, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN | LV_STATE_CHECKED);
  
  notificationLabel = lv_label_create(notificationButton);
  if (settingsController.GetNotificationStatus() == Controllers::Settings::Notification::ON) {
    lv_obj_add_state(notificationButton, LV_STATE_CHECKED);
    lv_label_set_text_static(notificationLabel, Symbols::notificationsOn);
  } else {
    lv_label_set_text_static(notificationLabel, Symbols::notificationsOff);
  }
  lv_obj_center(notificationLabel);

  
  settingsButton = lv_btn_create(container1);
  settingsButton->user_data = this;
  lv_obj_add_event_cb(settingsButton, ButtonEventHandler, LV_EVENT_ALL, settingsButton->user_data);
  lv_obj_add_style(settingsButton, &btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
  
  settingsLabel = lv_label_create(settingsButton);
  lv_label_set_text_static(settingsLabel, Symbols::settings);
  lv_obj_center(settingsLabel);

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
