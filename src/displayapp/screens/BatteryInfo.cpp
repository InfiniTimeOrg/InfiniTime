#include "BatteryInfo.h"
#include "../DisplayApp.h"
#include "components/battery/BatteryController.h"
#include "lv_i18n/lv_i18n.h"

using namespace Pinetime::Applications::Screens;

static void lv_update_task(struct _lv_task_t* task) {
  auto user_data = static_cast<BatteryInfo*>(task->user_data);
  user_data->UpdateScreen();
}

static void lv_anim_task(struct _lv_task_t* task) {
  auto user_data = static_cast<BatteryInfo*>(task->user_data);
  user_data->UpdateAnim();
}

BatteryInfo::BatteryInfo(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Battery& batteryController)
  : Screen(app), batteryController {batteryController} {

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  charging_bar = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(charging_bar, 200, 15);
  lv_bar_set_range(charging_bar, 0, 100);
  lv_obj_align(charging_bar, nullptr, LV_ALIGN_CENTER, 0, 10);
  lv_bar_set_anim_time(charging_bar, 2000);
  lv_obj_set_style_local_radius(charging_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x222222));
  lv_obj_set_style_local_bg_opa(charging_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_100);
  lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));
  lv_bar_set_value(charging_bar, batteryPercent, LV_ANIM_OFF);

  status = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(status, _("settings_battery_readingstatus"));
  lv_label_set_align(status, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  percent = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(percent, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  if (batteryPercent >= 0) {
    lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);
  } else {
    lv_label_set_text(percent, "--%");
  }
  lv_label_set_align(percent, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(percent, nullptr, LV_ALIGN_CENTER, 0, -60);

  // hack to not use the flot functions from printf
  uint8_t batteryVoltageBytes[2];
  batteryVoltageBytes[1] = static_cast<uint8_t>(batteryVoltage); // truncate whole numbers
  batteryVoltageBytes[0] =
    static_cast<uint8_t>((batteryVoltage - batteryVoltageBytes[1]) * 100); // remove whole part of flt and shift 2 places over
  //

  voltage = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(voltage, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xC6A600));
  lv_label_set_text_fmt(voltage, "%1i.%02i volts", batteryVoltageBytes[1], batteryVoltageBytes[0]);
  lv_label_set_align(voltage, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(voltage, nullptr, LV_ALIGN_CENTER, 0, 95);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  taskUpdate = lv_task_create(lv_update_task, 500000, LV_TASK_PRIO_LOW, this);
  taskAnim = lv_task_create(lv_anim_task, 1000, LV_TASK_PRIO_LOW, this);
  UpdateScreen();
}

BatteryInfo::~BatteryInfo() {
  lv_task_del(taskUpdate);
  lv_task_del(taskAnim);
  lv_obj_clean(lv_scr_act());
}

void BatteryInfo::UpdateAnim() {
  batteryPercent = batteryController.PercentRemaining();

  if (batteryPercent >= 0) {
    if (batteryController.IsCharging() and batteryPercent < 100) {
      animation += 1;
      if (animation >= 100) {
        animation = 0;
      }

    } else {
      if (animation > batteryPercent) {
        animation--;
      }
      if (animation < batteryPercent) {
        animation++;
      }
    }

    lv_bar_set_value(charging_bar, animation, LV_ANIM_OFF);
  }
}

void BatteryInfo::UpdateScreen() {

  batteryController.Update();

  batteryPercent = batteryController.PercentRemaining();
  batteryVoltage = batteryController.Voltage();

  if (batteryPercent >= 0) {
    if (batteryController.IsCharging() and batteryPercent < 100) {
      lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
      lv_label_set_text_static(status, _("settings_battery_charging"));
    } else if (batteryPercent == 100) {
      lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_BLUE);
      lv_label_set_text_static(status, _("settings_battery_fullycharged"));
    } else if (batteryPercent < 10) {
      lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
      lv_label_set_text_static(status, _("settings_battery_low"));
    } else {
      lv_obj_set_style_local_bg_color(charging_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_GREEN);
      lv_label_set_text_static(status, _("settings_battery_discharging"));
    }

    lv_label_set_text_fmt(percent, "%02i%%", batteryPercent);

  } else {
    lv_label_set_text_static(status, _("settings_battery_readingstatus"));
    lv_label_set_text(percent, "--%");
  }

  lv_obj_align(status, charging_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
  // hack to not use the flot functions from printf
  uint8_t batteryVoltageBytes[2];
  batteryVoltageBytes[1] = static_cast<uint8_t>(batteryVoltage); // truncate whole numbers
  batteryVoltageBytes[0] =
    static_cast<uint8_t>((batteryVoltage - batteryVoltageBytes[1]) * 100); // remove whole part of flt and shift 2 places over
  //
  lv_label_set_text_fmt(voltage, _("settings_battery_volts"), batteryVoltageBytes[1], batteryVoltageBytes[0]);
}

bool BatteryInfo::Refresh() {

  return running;
}
