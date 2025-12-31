#include "displayapp/screens/HeartRateZone.h"
#include <lvgl/lvgl.h>
#include <components/heartrate/HeartRateController.h>

#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

HeartRateZone::HeartRateZone(Controllers::HeartRateController& heartRateController, System::SystemTask& systemTask)
  : heartRateController {heartRateController}, wakeLock(systemTask) {
  auto activity = heartRateController.Activity();
  auto settings = heartRateController.hrzSettings();
  uint32_t total = 0;

  auto hundreths_of_hour = pdMS_TO_TICKS(10 * 60 * 60);
  auto exercise_target = pdMS_TO_TICKS(settings.exerciseMsTarget);
  uint32_t offset = 25 * (zone_bar.size() + 2);

  lv_obj_t* screen = lv_scr_act();

  title = lv_label_create(screen, nullptr);
  lv_label_set_text_static(title, "BPM Breakdown");
  lv_obj_align(title, screen, LV_ALIGN_IN_TOP_MID, 0, 20);

  total_bar = lv_bar_create(screen, nullptr);
  lv_obj_set_style_local_bg_opa(total_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_line_color(total_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, Colors::bgAlt);
  lv_obj_set_style_local_border_width(total_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_radius(total_bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_line_color(total_bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_NAVY);

  lv_obj_align(total_bar, screen, LV_ALIGN_IN_TOP_MID, 0, offset - (zone_bar.size() * 25));

  total_label = lv_label_create(total_bar, nullptr);
  lv_obj_align(total_label, total_bar, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_text_color(total_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);

  for (uint8_t i = 0; i < zone_bar.size(); i++) {
    zone_bar[i] = lv_bar_create(screen, nullptr);

    total += activity.zoneTime[i];

    lv_obj_set_style_local_bg_opa(zone_bar[i], LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
    lv_obj_set_style_local_line_color(zone_bar[i], LV_BAR_PART_BG, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_style_local_border_width(zone_bar[i], LV_BAR_PART_BG, LV_STATE_DEFAULT, 2);
    lv_obj_set_style_local_radius(zone_bar[i], LV_BAR_PART_BG, LV_STATE_DEFAULT, 0);

    lv_obj_set_size(zone_bar[i], 240, 20);
    lv_obj_align(zone_bar[i], screen, LV_ALIGN_IN_TOP_MID, 0, offset - i * 25);

    label_time[i] = lv_label_create(zone_bar[i], nullptr);
    lv_obj_align(label_time[i], zone_bar[i], LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_local_text_color(label_time[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  }

  lv_label_set_text_static(label_time[0], "Warm Up");
  lv_label_set_text_static(label_time[1], "Recovery");
  lv_label_set_text_static(label_time[2], "Aerobic");
  lv_label_set_text_static(label_time[3], "Threshold");
  lv_label_set_text_static(label_time[4], "Anaerobic");
  lv_label_set_text_static(total_label, "Goal");

  lv_obj_set_style_local_line_color(zone_bar[0], LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::blue);
  lv_obj_set_style_local_line_color(zone_bar[1], LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::green);
  lv_obj_set_style_local_line_color(zone_bar[2], LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::orange);
  lv_obj_set_style_local_line_color(zone_bar[3], LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::deepOrange);
  lv_obj_set_style_local_line_color(zone_bar[4], LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Colors::heartRed);

  auto bar_limit = total / hundreths_of_hour;

  for (uint8_t i = 0; i < zone_bar.size(); i++) {
    uint32_t percent = activity.zoneTime[i] / hundreths_of_hour;
    lv_bar_set_range(zone_bar[i], 0, bar_limit);
    lv_bar_set_value(zone_bar[i], percent, LV_ANIM_OFF);
  }

  lv_bar_set_range(total_bar, 0, exercise_target);
  lv_bar_set_value(total_bar, total > exercise_target ? exercise_target : total, LV_ANIM_OFF);

  taskRefresh = lv_task_create(RefreshTaskCallback, 5000, LV_TASK_PRIO_MID, this);
}

HeartRateZone::~HeartRateZone() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void HeartRateZone::Refresh() {
  auto activity = heartRateController.Activity();
  auto settings = heartRateController.hrzSettings();
  uint32_t total = 0;

  auto hundreths_of_hour = pdMS_TO_TICKS(10 * 60 * 60);
  auto exercise_target = pdMS_TO_TICKS(settings.exerciseMsTarget);

  for (uint8_t i = 0; i < zone_bar.size(); i++) {
    total += activity.zoneTime[i];
  }

  auto bar_limit = total / hundreths_of_hour;

  for (uint8_t i = 0; i < zone_bar.size(); i++) {
    uint32_t percent = activity.zoneTime[i] / hundreths_of_hour;
    lv_bar_set_range(zone_bar[i], 0, bar_limit);
    lv_bar_set_value(zone_bar[i], percent, LV_ANIM_OFF);
  }

  lv_bar_set_range(total_bar, 0, exercise_target);
  lv_bar_set_value(total_bar, total > exercise_target ? exercise_target : total, LV_ANIM_OFF);
}