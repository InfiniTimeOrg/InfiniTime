#include "Tile.h"
#include "../DisplayApp.h"
#include "BatteryIcon.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void lv_update_task(struct _lv_timer_t* task) {
    auto* user_data = static_cast<Tile*>(task->user_data);
    user_data->UpdateScreen();
  }

  static void event_handler(lv_event_t* event) {
    if (lv_event_get_code(event) != LV_EVENT_VALUE_CHANGED) return;

    Tile* screen = static_cast<Tile*>(lv_event_get_user_data(event));
    lv_obj_t *obj = lv_event_get_target(event);
    uint32_t btnId= lv_btnmatrix_get_selected_btn(obj);
    screen->OnValueChangedEvent(obj, btnId);
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
  label_time = lv_label_create(lv_scr_act());
  lv_label_set_text_fmt(label_time, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_obj_set_align(label_time, LV_ALIGN_CENTER);
  lv_obj_align(label_time, LV_ALIGN_TOP_LEFT, 0, 0);

  // Battery
  batteryIcon = lv_label_create(lv_scr_act());
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
  lv_obj_align(batteryIcon, LV_ALIGN_TOP_RIGHT, -8, 0);

  if (numScreens > 1) {
    pageIndicatorBasePoints[0].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[0].y = 0;
    pageIndicatorBasePoints[1].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[1].y = LV_VER_RES;

    pageIndicatorBase = lv_line_create(lv_scr_act());
    lv_obj_set_style_line_width(pageIndicatorBase, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(pageIndicatorBase, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

    const uint16_t indicatorSize = LV_VER_RES / numScreens;
    const uint16_t indicatorPos = indicatorSize * screenID;

    pageIndicatorPoints[0].x = LV_HOR_RES - 1;
    pageIndicatorPoints[0].y = indicatorPos;
    pageIndicatorPoints[1].x = LV_HOR_RES - 1;
    pageIndicatorPoints[1].y = indicatorPos + indicatorSize;

    pageIndicator = lv_line_create(lv_scr_act());
    lv_obj_set_style_line_width(pageIndicator, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(pageIndicator, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
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

  btnm1 = lv_btnmatrix_create(lv_scr_act());
  lv_btnmatrix_set_map(btnm1, btnmMap);
  lv_obj_set_size(btnm1, LV_HOR_RES - 16, LV_VER_RES - 60);
  lv_obj_align(btnm1, LV_ALIGN_CENTER, 0, 10);

  lv_obj_set_style_radius(btnm1, 20, LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(btnm1, LV_OPA_20, LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(btnm1, lv_color_hex(0x00FFFF), LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(btnm1, LV_OPA_20, LV_PART_ITEMS | LV_STATE_DISABLED);
  lv_obj_set_style_bg_color(btnm1, lv_color_hex(0x111111), LV_PART_ITEMS | LV_STATE_DISABLED);
  lv_obj_set_style_pad_all(btnm1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_gap(btnm1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

  for (uint8_t i = 0; i < 6; i++) {
    lv_btnmatrix_set_btn_ctrl(btnm1, i, LV_BTNMATRIX_CTRL_CLICK_TRIG);
    if (applications[i].application == Apps::None) {
      lv_btnmatrix_set_btn_ctrl(btnm1, i, LV_BTNMATRIX_CTRL_DISABLED);
    }
  }

  btnm1->user_data = this;
  lv_obj_add_event_cb(btnm1, event_handler, LV_EVENT_ALL, btnm1->user_data);

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  taskUpdate = lv_timer_create(lv_update_task, 5000, this);
}

Tile::~Tile() {
  lv_timer_del(taskUpdate);
  lv_obj_clean(lv_scr_act());
}

void Tile::UpdateScreen() {
  lv_label_set_text_fmt(label_time, "%02i:%02i", dateTimeController.Hours(), dateTimeController.Minutes());
  lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryController.PercentRemaining()));
}

void Tile::OnValueChangedEvent(lv_obj_t* obj, uint32_t buttonId) {
  if(obj != btnm1) return;

  app->StartApp(apps[buttonId], DisplayApp::FullRefreshDirections::Up);
  running = false;
}
