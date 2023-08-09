#include "displayapp/screens/FirmwareUpdate.h"
#include <lvgl/lvgl.h>
#include "components/ble/BleController.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

FirmwareUpdate::FirmwareUpdate(const Pinetime::Controllers::Ble& bleController) : bleController {bleController} {

  titleLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(titleLabel, "Firmware update");
  lv_obj_align(titleLabel, nullptr, LV_ALIGN_IN_TOP_MID, 0, 50);

//  // Create the white rectangle (outer border)
//  whiteRect = lv_obj_create(lv_scr_act(), nullptr);
//  lv_obj_set_size(whiteRect, 204, 34);  // Two pixels larger in each direction
//  lv_obj_align(whiteRect, bar1, LV_ALIGN_CENTER, 0, 0);
//  lv_obj_set_style_local_bg_color(whiteRect, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
//
//  // Create the black rectangle (inner border)
//  blackRect = lv_obj_create(lv_scr_act(), nullptr);
//  lv_obj_set_size(blackRect, 200, 30);  // Same size as the original bar
//  lv_obj_align(blackRect, bar1, LV_ALIGN_CENTER, 0, 0);
//  lv_obj_set_style_local_bg_color(blackRect, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  bar1 = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(bar1, 204, 34);
  lv_obj_align(bar1, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_bar_set_range(bar1, 0, 1);
  lv_bar_set_value(bar1, 1, LV_ANIM_OFF);
  lv_obj_set_style_local_bg_color(bar1, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  bar2 = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(bar1, 200, 30);
  lv_obj_align(bar1, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_bar_set_range(bar1, 0, 1);
  lv_bar_set_value(bar1, 1, LV_ANIM_OFF);
  lv_obj_set_style_local_bg_color(bar2, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  bar3 = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(bar1, 200, 30);
  lv_obj_align(bar1, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_bar_set_range(bar1, 0, 1000);
  lv_bar_set_value(bar1, 0, LV_ANIM_OFF);

  percentLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(percentLabel, "Waiting...");
  lv_label_set_recolor(percentLabel, true);
  lv_obj_set_auto_realign(percentLabel, true);
  lv_obj_align(percentLabel, bar1, LV_ALIGN_OUT_TOP_MID, 0, 60);
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  startTime = xTaskGetTickCount();

 // // Make sure the bar is on top of the two rectangles
 // lv_obj_move_foreground(bar1);
 //
 // // Adjust the z-index of the rectangles to ensure proper layering
 // lv_obj_move_background(whiteRect);
 // lv_obj_move_background(blackRect);
}

FirmwareUpdate::~FirmwareUpdate() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void FirmwareUpdate::Refresh() {
  switch (bleController.State()) {
    default:
    case Pinetime::Controllers::Ble::FirmwareUpdateStates::Idle:
      // This condition makes sure that the app is exited if somehow it got
      // launched without a firmware update. This should never happen.
      if (state != States::Error) {
        if (xTaskGetTickCount() - startTime > (60 * 1024)) {
          UpdateError();
          state = States::Error;
        }
      } else if (xTaskGetTickCount() - startTime > (5 * 1024)) {
        running = false;
      }
      break;
    case Pinetime::Controllers::Ble::FirmwareUpdateStates::Running:
      if (state != States::Running) {
        state = States::Running;
      }
      DisplayProgression();
      break;
    case Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated:
      if (state != States::Validated) {
        UpdateValidated();
        state = States::Validated;
      }
      break;
    case Pinetime::Controllers::Ble::FirmwareUpdateStates::Error:
      if (state != States::Error) {
        UpdateError();
        state = States::Error;
      }
      if (xTaskGetTickCount() - startTime > (5 * 1024)) {
        running = false;
      }
      break;
  }
}

void FirmwareUpdate::DisplayProgression() const {
  const uint32_t current = bleController.FirmwareUpdateCurrentBytes();
  const uint32_t total   = bleController.FirmwareUpdateTotalBytes();
  const int16_t permille = current / (total / 1000);

  lv_label_set_text_fmt(percentLabel, "%d %%", permille / 10);

  lv_color_t color = lv_color_hsv_to_rgb(( permille / 10 * 1.2 ), 100, 100);

  lv_bar_set_value(bar3, permille, LV_ANIM_OFF);
  lv_obj_set_style_local_bg_color(bar3, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, color);
}

void FirmwareUpdate::UpdateValidated() {
  lv_label_set_text_static(percentLabel, "#00ff00 Image Ok!#");
}

void FirmwareUpdate::UpdateError() {
  lv_label_set_text_static(percentLabel, "#ff0000 Error!#");
  lv_obj_set_style_local_bg_color(bar3, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_hsv_to_rgb(0, 100, 100));
  startTime = xTaskGetTickCount();
}

bool FirmwareUpdate::OnButtonPushed() {
  return true;
}
