#include "displayapp/screens/PomodoroTimer.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

PomodoroTimer::PomodoroTimer(AppControllers& controllers) : controllers(controllers) {
  // Create main time display
  lblTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(lblTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(lblTime, "25:00");
  lv_obj_align(lblTime, lv_scr_act(), LV_ALIGN_CENTER, 0, -40);

  // Create session type label
  lblSessionType = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lblSessionType, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_label_set_text_static(lblSessionType, "Work Session");
  lv_obj_align(lblSessionType, lv_scr_act(), LV_ALIGN_CENTER, 0, -80);

  // Create start/pause button
  btnStartPause = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btnStartPause, 120, 50);
  lv_obj_align(btnStartPause, lv_scr_act(), LV_ALIGN_CENTER, 0, 60);

  lblStartPause = lv_label_create(btnStartPause, nullptr);
  lv_label_set_text_static(lblStartPause, "Start");
  lv_obj_align(lblStartPause, btnStartPause, LV_ALIGN_CENTER, 0, 0);
}

PomodoroTimer::~PomodoroTimer() {
  lv_obj_clean(lv_scr_act());
}

void PomodoroTimer::Refresh() {
  // Placeholder for future timer update logic
}