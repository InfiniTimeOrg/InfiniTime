#include "Tile.h"
#include "../DisplayApp.h"
#include "BatteryIcon.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void lv_update_task(struct _lv_task_t* task) {
    auto user_data = static_cast<Tile*>(task->user_data);
    user_data->UpdateScreen();
  }

  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    Tile* screen = static_cast<Tile*>(obj->user_data);
    uint32_t* eventDataPtr = (uint32_t*) lv_event_get_data();
    uint32_t eventData = *eventDataPtr;
    screen->OnObjectEvent(obj, event, eventData);
  }
}

Tile::Tile(uint8_t screenID,
           uint8_t numScreens,
           DisplayApp* app,
           Controllers::Settings& settingsController,
           Pinetime::Controllers::Battery& batteryController,
           Controllers::DateTime& dateTimeController,
           std::array<Applications, 6>& applications)
  : Screen(app), batteryController {batteryController}, dateTimeController {dateTimeController} {

  settingsController.SetAppMenu(screenID);

  // Time
  label_time = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(label_time, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_label_set_align(label_time, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_time, nullptr, LV_ALIGN_IN_TOP_LEFT, 15, 6);

  // Battery
  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
  lv_obj_align(batteryIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, -15, 6);

  if (numScreens > 1) {
    pageIndicatorBasePoints[0].x = 240 - 1;
    pageIndicatorBasePoints[0].y = 6;
    pageIndicatorBasePoints[1].x = 240 - 1;
    pageIndicatorBasePoints[1].y = 240 - 6;

    pageIndicatorBase = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_line_width(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
    lv_obj_set_style_local_line_rounded(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
    lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

    uint16_t indicatorSize = 228 / numScreens;
    uint16_t indicatorPos = indicatorSize * screenID;

    pageIndicatorPoints[0].x = 240 - 1;
    pageIndicatorPoints[0].y = 6 + indicatorPos;
    pageIndicatorPoints[1].x = 240 - 1;
    pageIndicatorPoints[1].y = 6 + indicatorPos + indicatorSize;

    pageIndicator = lv_line_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_line_width(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_line_rounded(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
    lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
  }

  uint8_t btIndex = 0;
  for (uint8_t i = 0; i < 6; i++) {
    if (i == 3)
      btnmMap[btIndex++] = "\n";
    if (applications[i].application == Apps::None) {
      btnmMap[btIndex] = " ";
    } else {
      btnmMap[btIndex] = applications[i].icon;
    }
    btIndex++;
    apps[i] = applications[i].application;
  }
  btnmMap[btIndex] = "";

  btnm1 = lv_btnmatrix_create(lv_scr_act(), nullptr);
  lv_btnmatrix_set_map(btnm1, btnmMap);
  lv_obj_set_size(btnm1, LV_HOR_RES - 10, LV_VER_RES - 60);
  lv_obj_align(btnm1, NULL, LV_ALIGN_CENTER, 0, 10);

  lv_obj_set_style_local_radius(btnm1, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_opa(btnm1, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_OPA_20);
  lv_obj_set_style_local_bg_color(btnm1, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  lv_obj_set_style_local_bg_opa(btnm1, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED, LV_OPA_20);
  lv_obj_set_style_local_bg_color(btnm1, LV_BTNMATRIX_PART_BTN, LV_STATE_DISABLED, lv_color_hex(0x111111));

  for (uint8_t i = 0; i < 6; i++) {
    if (applications[i].application == Apps::None) {
      lv_btnmatrix_set_btn_ctrl(btnm1, i, LV_BTNMATRIX_CTRL_DISABLED);
    }
  }

  btnm1->user_data = this;
  lv_obj_set_event_cb(btnm1, event_handler);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  taskUpdate = lv_task_create(lv_update_task, 5000, LV_TASK_PRIO_MID, this);
}

Tile::~Tile() {
  lv_task_del(taskUpdate);
  lv_obj_clean(lv_scr_act());
}

void Tile::UpdateScreen() {
  lv_label_set_text_fmt(label_time, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
}

bool Tile::Refresh() {
  return running;
}

void Tile::OnObjectEvent(lv_obj_t* obj, lv_event_t event, uint32_t buttonId) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    app->StartApp(apps[buttonId], DisplayApp::FullRefreshDirections::Up);
    running = false;
  }
}
