#include "HoneyComb.h"
#include "../DisplayApp.h"
#include "BatteryIcon.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void UpdateTaskCallback(struct _lv_task_t* task) {
    auto* user_data = static_cast<HoneyComb*>(task->user_data);
    user_data->UpdateScreen();
  }

  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<HoneyComb*>(obj->user_data);
    screen->OnObjectEvent(obj, event);
  }
}

HoneyComb::HoneyComb(uint8_t screenID,
                     uint8_t numScreens,
                     DisplayApp* app,
                     Controllers::Settings& settingsController,
                     Pinetime::Controllers::Battery& batteryController,
                     Controllers::DateTime& dateTimeController,
                     std::array<Applications, 8>& applications)
  : Screen(app), batteryController {batteryController}, dateTimeController {dateTimeController} {

  settingsController.SetAppMenu(screenID);

  labelTime = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(labelTime, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_obj_align(labelTime, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
  lv_obj_align(batteryIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, -8, 0);

  if (numScreens > 1) {
    pageIndicatorBasePoints[0].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[0].y = 0;
    pageIndicatorBasePoints[1].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[1].y = LV_VER_RES;

    pageIndicatorBase = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_line_width(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
    lv_obj_set_style_local_line_rounded(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
    lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

    const uint16_t indicatorSize = LV_VER_RES / numScreens;
    const uint16_t indicatorPos = indicatorSize * screenID;

    pageIndicatorPoints[0].x = LV_HOR_RES - 1;
    pageIndicatorPoints[0].y = indicatorPos;
    pageIndicatorPoints[1].x = LV_HOR_RES - 1;
    pageIndicatorPoints[1].y = indicatorPos + indicatorSize;

    pageIndicator = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_line_width(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_line_rounded(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
    lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
  }

  for (uint8_t i = 0; i < 8; i++) {
    buttons[i] = lv_btn_create(lv_scr_act(), nullptr);

    lv_obj_set_style_local_radius(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
    lv_obj_set_style_local_bg_opa(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
    lv_obj_set_style_local_bg_color(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
    lv_obj_set_style_local_bg_opa(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DISABLED, LV_OPA_20);
    lv_obj_set_style_local_bg_color(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DISABLED, lv_color_hex(0x111111));
    lv_obj_set_size(buttons[i], 70, 70);

    apps[i] = applications[i].application;

    if (applications[i].application == Apps::None) {
      lv_obj_set_state(buttons[i], LV_STATE_DISABLED);
    } else {
      lv_obj_set_style_local_value_str(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, applications[i].icon);
      lv_obj_set_event_cb(buttons[i], EventHandler);
      buttons[i]->user_data = this;
    }
  }

  lv_obj_set_pos(buttons[0],   8,  27);
  lv_obj_set_pos(buttons[1],  85,  27);
  lv_obj_set_pos(buttons[2], 162,  27);
  lv_obj_set_pos(buttons[3],  46,  92);
  lv_obj_set_pos(buttons[4], 124,  92);
  lv_obj_set_pos(buttons[5],   8, 157);
  lv_obj_set_pos(buttons[6],  85, 157);
  lv_obj_set_pos(buttons[7], 162, 157);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  taskUpdate = lv_task_create(UpdateTaskCallback, 5000, LV_TASK_PRIO_MID, this);
}

HoneyComb::~HoneyComb() {
  lv_task_del(taskUpdate);
  lv_obj_clean(lv_scr_act());
}

void HoneyComb::UpdateScreen() {
  lv_label_set_text_fmt(labelTime, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
}

bool HoneyComb::Refresh() {
  return running;
}

void HoneyComb::OnObjectEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }

  for (uint8_t i = 0; i < 8; i++) {
    if (buttons[i] == obj) {
      app->StartApp(apps[i], DisplayApp::FullRefreshDirections::Up);
    }
  }
}
