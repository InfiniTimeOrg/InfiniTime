#include "SettingShakeThreshold.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    SettingShakeThreshold* screen = static_cast<SettingShakeThreshold*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

SettingShakeThreshold::SettingShakeThreshold(DisplayApp* app,
                                             Controllers::Settings& settingsController,
                                             Controllers::MotionController& motionController,
                                             System::SystemTask& systemTask)
  : Screen(app), settingsController {settingsController}, motionController {motionController}, systemTask {systemTask} {

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Wake Sensitivity");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  taskCount = 0;

  positionArc = lv_arc_create(lv_scr_act(), nullptr);

  positionArc->user_data = this;

  lv_obj_set_event_cb(positionArc, event_handler);
  // lv_arc_set_start_angle(positionArc,270);
  lv_arc_set_angles(positionArc, 180, 360);
  lv_arc_set_bg_angles(positionArc, 180, 360);

  // lv_arc_set_rotation(positionArc, 135);
  lv_arc_set_range(positionArc, 0, 4095);
  lv_arc_set_value(positionArc, settingsController.GetShakeThreshold());
  lv_obj_set_size(positionArc, 240, 180);
  lv_arc_set_adjustable(positionArc, true);
  lv_obj_align(positionArc, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

  calButton = lv_btn_create(lv_scr_act(), nullptr);
  calButton->user_data = this;
  lv_obj_set_event_cb(calButton, event_handler);
  lv_btn_set_fit(calButton, LV_FIT_TIGHT);

  // lv_obj_set_style_local_bg_opa(calButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_height(calButton, 80);
  lv_obj_align(calButton, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_btn_set_checkable(calButton, true);
  calLabel = lv_label_create(calButton, NULL);
  lv_label_set_text(calLabel, "Calibrate");
}

SettingShakeThreshold::~SettingShakeThreshold() {
  settingsController.SetShakeThreshold(lv_arc_get_value(positionArc));
  if (taskCount > 0) {
    lv_task_del(refreshTask);
  }
  settingsController.SaveSettings();
  lv_obj_clean(lv_scr_act());
}

void SettingShakeThreshold::Refresh() {

  taskCount++; // 100ms Per update
  if ((motionController.currentShakeSpeed() - 300) > lv_arc_get_value(positionArc)) {
    lv_arc_set_value(positionArc, (int16_t) motionController.currentShakeSpeed() - 200);
  }
  if (taskCount >= 75) {
    lv_btn_set_state(calButton,LV_STATE_DEFAULT);
    lv_event_send(calButton,LV_EVENT_VALUE_CHANGED,NULL);
    taskCount = 0;
    lv_task_del(refreshTask);
  }
}

void SettingShakeThreshold::UpdateSelected(lv_obj_t* object, lv_event_t event) {

  switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
      if (object == positionArc) {
        settingsController.SetShakeThreshold(lv_arc_get_value(positionArc));
        break;
      }
      if (object == calButton) {
        if (lv_btn_get_state(calButton) == LV_BTN_STATE_CHECKED_RELEASED) {
          lv_arc_set_value(positionArc, 0);
          refreshTask = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
          lv_label_set_text(calLabel, "Shake!!!");

        } else if (lv_btn_get_state(calButton) == LV_BTN_STATE_RELEASED) {

          lv_task_del(refreshTask);
          taskCount = 0;
          lv_label_set_text(calLabel, "Calibrate");
        }
        break;
      }
    }
  }
}
