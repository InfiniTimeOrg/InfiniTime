#include "displayapp/screens/FirmwareUpdate.h"
#include <lvgl/lvgl.h>
#include "components/ble/BleController.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

FirmwareUpdate::FirmwareUpdate(const Pinetime::Controllers::Ble& bleController) : bleController {bleController} {

  titleLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(titleLabel, "Firmware update");
  lv_obj_align(titleLabel, nullptr, LV_ALIGN_IN_TOP_MID, 0, 50);

  bar1 = lv_bar_create(lv_scr_act(), nullptr);
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
  const uint32_t total = bleController.FirmwareUpdateTotalBytes();
  const int16_t permille = current / (total / 1000);

  lv_label_set_text_fmt(percentLabel, "%d %%", permille / 10);

  lv_bar_set_value(bar1, permille, LV_ANIM_OFF);
}

void FirmwareUpdate::UpdateValidated() {
  lv_label_set_text_static(percentLabel, "#00ff00 Image Ok!#");
}

void FirmwareUpdate::UpdateError() {
  lv_label_set_text_static(percentLabel, "#ff0000 Error!#");
  startTime = xTaskGetTickCount();
}

bool FirmwareUpdate::OnButtonPushed() {
  return true;
}
