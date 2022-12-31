#include "displayapp/screens/BatteryInfo.h"
#include "displayapp/DisplayApp.h"
#include "components/battery/BatteryController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

BatteryInfo::BatteryInfo(const Pinetime::Controllers::Battery& batteryController) : batteryController {batteryController} {

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  charging_bar = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(charging_bar, 200, 15);
  lv_bar_set_range(charging_bar, 0, 100);
  lv_obj_align(charging_bar, nullptr, LV_ALIGN_CENTER, 0, 10);
  lv_bar_set_anim_time(charging_bar, 1000);
  lv_obj_set_style_local_radius(charging_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_bg_opa(charging_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
  lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_bar_set_value(charging_bar, batteryPercent, LV_ANIM_ON);

  status = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(status, "Reading Battery status");
  lv_label_set_align(status, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  percent = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(percent, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);
  lv_label_set_align(percent, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(percent, nullptr, LV_ALIGN_CENTER, 0, -60);

  voltage = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(voltage, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_label_set_align(voltage, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(voltage, nullptr, LV_ALIGN_CENTER, 0, 95);

  taskRefresh = lv_task_create(RefreshTaskCallback, 5000, LV_TASK_PRIO_MID, this);
  Refresh();
}

BatteryInfo::~BatteryInfo() {
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
    lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::highlight);
    lv_label_set_text_static(status, "Discharging");
  }

  lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);

  lv_obj_align(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_bar_set_value(charging_bar, batteryPercent, LV_ANIM_ON);
}
