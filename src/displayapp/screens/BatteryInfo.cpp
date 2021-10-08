#include "BatteryInfo.h"
#include "../DisplayApp.h"
#include "components/battery/BatteryController.h"

using namespace Pinetime::Applications::Screens;

BatteryInfo::BatteryInfo(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Battery& batteryController)
  : Screen(app), batteryController {batteryController} {

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  charging_bar = lv_bar_create(lv_scr_act());
  lv_obj_set_size(charging_bar, 200, 15);
  lv_bar_set_range(charging_bar, 0, 100);
  lv_obj_align(charging_bar,  LV_ALIGN_CENTER, 0, 10);
  lv_obj_set_style_anim_time(charging_bar, 1000, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(charging_bar, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(charging_bar, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(charging_bar, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(charging_bar, lv_color_hex(0xFF0000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_bar_set_value(charging_bar, batteryPercent, LV_ANIM_ON);

  status = lv_label_create(lv_scr_act());
  lv_label_set_text_static(status, "Reading Battery status");
  lv_obj_set_style_text_align(status, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align_to(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  percent = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(percent, &jetbrains_mono_76, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);
  lv_obj_set_style_text_align(percent, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(percent,  LV_ALIGN_CENTER, 0, -60);

  voltage = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(voltage, lv_color_hex(0xC6A600), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_obj_set_style_text_align(voltage, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(voltage,  LV_ALIGN_CENTER, 0, 95);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  taskRefresh = lv_timer_create(RefreshTaskCallback, 5000, this);
  Refresh();
}

BatteryInfo::~BatteryInfo() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void BatteryInfo::Refresh() {

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  if (batteryController.IsCharging()) {
    lv_obj_set_style_bg_color(charging_bar, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_label_set_text_static(status, "Charging");
  } else if (batteryPercent == 100) {
    lv_obj_set_style_bg_color(charging_bar, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_label_set_text_static(status, "Fully charged");
  } else if (batteryPercent < 10) {
    lv_obj_set_style_bg_color(charging_bar, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_label_set_text_static(status, "Battery low");
  } else {
    lv_obj_set_style_bg_color(charging_bar, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_label_set_text_static(status, "Discharging");
  }

  lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);

  lv_obj_align_to(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_bar_set_value(charging_bar, batteryPercent, LV_ANIM_ON);
}
