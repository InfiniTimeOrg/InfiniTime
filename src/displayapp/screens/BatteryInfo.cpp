#include "displayapp/screens/BatteryInfo.h"
#include "displayapp/DisplayApp.h"
#include "components/battery/BatteryController.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void event_handler(lv_obj_t* obj, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
      auto* battery = static_cast<BatteryInfo*>(obj->user_data);
      battery->ToggleBatteryPercentState();
    }
  }
}

BatteryInfo::BatteryInfo(Pinetime::Applications::DisplayApp* app,
                         Pinetime::Controllers::Battery& batteryController,
                         Pinetime::Controllers::Settings& settingsController)
  : Screen(app), batteryController {batteryController}, settingsController {settingsController} {

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  charging_bar = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(charging_bar, 200, 15);
  lv_bar_set_range(charging_bar, 0, 100);
  lv_obj_align(charging_bar, nullptr, LV_ALIGN_CENTER, 0, -30);
  lv_bar_set_anim_time(charging_bar, 1000);
  lv_obj_set_style_local_radius(charging_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x222222));
  lv_obj_set_style_local_bg_opa(charging_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
  lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));
  lv_bar_set_value(charging_bar, batteryPercent, LV_ANIM_ON);

  status = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(status, "Reading Battery status");
  lv_label_set_align(status, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  percent = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(percent, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);
  lv_label_set_align(percent, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(percent, nullptr, LV_ALIGN_CENTER, 0, -70);

  voltage = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(voltage, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xff, 0xb0, 0x0));
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_label_set_align(voltage, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(voltage, nullptr, LV_ALIGN_CENTER, 0, 55);

  show_percentage_checkbox = lv_checkbox_create(lv_scr_act(), nullptr);
  lv_checkbox_set_text(show_percentage_checkbox, "Show %");
  lv_obj_align(show_percentage_checkbox, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
  lv_obj_add_state(show_percentage_checkbox, LV_STATE_DEFAULT);
  lv_checkbox_set_checked(show_percentage_checkbox, settingsController.GetShowBatteryPercentage());
  show_percentage_checkbox->user_data = this;
  lv_obj_set_event_cb(show_percentage_checkbox, event_handler);

  taskRefresh = lv_task_create(RefreshTaskCallback, 5000, LV_TASK_PRIO_MID, this);
  Refresh();
}

BatteryInfo::~BatteryInfo() {
  settingsController.SaveSettings();
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void BatteryInfo::Refresh() {

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  if (batteryController.IsCharging()) {
    lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_label_set_text_static(status, "Charging");
  } else if (batteryPercent == 100) {
    lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    lv_label_set_text_static(status, "Fully charged");
  } else if (batteryPercent < 10) {
    lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
    lv_label_set_text_static(status, "Battery low");
  } else {
    lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x0, 0xb0, 0x0));
    lv_label_set_text_static(status, "Discharging");
  }

  lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);

  lv_obj_align(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_bar_set_value(charging_bar, batteryPercent, LV_ANIM_ON);
}

void BatteryInfo::ToggleBatteryPercentState() {
  settingsController.SetShowBatteryPercentage(!settingsController.GetShowBatteryPercentage());
  lv_checkbox_set_checked(show_percentage_checkbox, settingsController.GetShowBatteryPercentage());
}
