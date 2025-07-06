#include "displayapp/screens/BatteryInfo.h"
#include "displayapp/DisplayApp.h"
#include "components/battery/BatteryController.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

BatteryInfo::BatteryInfo(const Pinetime::Controllers::Battery& batteryController) : batteryController {batteryController} {

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  chargingArc = lv_arc_create(lv_scr_act(), nullptr);
  lv_arc_set_rotation(chargingArc, 270);
  lv_arc_set_bg_angles(chargingArc, 0, 360);
  lv_arc_set_adjustable(chargingArc, false);
  lv_obj_set_size(chargingArc, 180, 180);
  lv_obj_align(chargingArc, nullptr, LV_ALIGN_CENTER, 0, -30);
  lv_arc_set_value(chargingArc, batteryPercent);
  lv_obj_set_style_local_bg_opa(chargingArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_line_color(chargingArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_border_width(chargingArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_radius(chargingArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_line_color(chargingArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_LIME);

  status = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(status, "Reading Battery status");
  lv_label_set_align(status, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(status, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -17);

  percent = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(percent, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(percent, "%i%%", batteryPercent);
  lv_label_set_align(percent, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(percent, chargingArc, LV_ALIGN_CENTER, 0, 0);

  voltage = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(voltage, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::orange);
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_label_set_align(voltage, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(voltage, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -7);

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
    lv_obj_set_style_local_line_color(chargingArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_LIME);
    lv_label_set_text_static(status, "Charging");
  } else if (batteryPercent == 100) {
    lv_obj_set_style_local_line_color(chargingArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    lv_label_set_text_static(status, "Fully charged");
  } else if (batteryPercent > 15) {
    lv_obj_set_style_local_line_color(chargingArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    lv_label_set_text_static(status, "Discharging");
  } else if (batteryPercent > 5) {
    lv_obj_set_style_local_line_color(chargingArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
    lv_label_set_text_static(status, "Battery low");
  } else {
    lv_obj_set_style_local_line_color(chargingArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, Colors::deepOrange);
    lv_label_set_text_static(status, "Battery critical");
  }

  lv_label_set_text_fmt(percent, "%i%%", batteryPercent);
  lv_obj_align(percent, chargingArc, LV_ALIGN_CENTER, 0, 0);

  lv_obj_align(status, voltage, LV_ALIGN_IN_BOTTOM_MID, 0, -27);
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltage / 1000, batteryVoltage % 1000 / 10);
  lv_arc_set_value(chargingArc, batteryPercent);
}
