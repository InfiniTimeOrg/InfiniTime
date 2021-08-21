#include "List.h"
#include "../DisplayApp.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void ButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    List* screen = static_cast<List*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }

}

List::List(uint8_t screenID,
           uint8_t numScreens,
           DisplayApp* app,
           Controllers::Settings& settingsController,
           std::array<Applications, MAXLISTITEMS>& applications)
  : Screen(app), settingsController {settingsController} {

  // Set the background to Black
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));

  settingsController.SetSettingsMenu(screenID);

  if (numScreens > 1) {
    pageIndicatorBasePoints[0].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[0].y = 0;
    pageIndicatorBasePoints[1].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[1].y = LV_VER_RES;

    pageIndicatorBase = lv_line_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_line_width(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
    lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

    const uint16_t indicatorSize = LV_VER_RES / numScreens;
    const uint16_t indicatorPos = indicatorSize * screenID;

    pageIndicatorPoints[0].x = LV_HOR_RES - 1;
    pageIndicatorPoints[0].y = indicatorPos;
    pageIndicatorPoints[1].x = LV_HOR_RES - 1;
    pageIndicatorPoints[1].y = indicatorPos + indicatorSize;

    pageIndicator = lv_line_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_line_width(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
  }

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 0, 0);
  lv_obj_set_width(container1, LV_HOR_RES - 8);
  lv_obj_set_height(container1, LV_VER_RES);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* labelBt;
  lv_obj_t* labelBtIco;

  for (int i = 0; i < MAXLISTITEMS; i++) {
    apps[i] = applications[i].application;
    if (applications[i].application != Apps::None) {

      itemApps[i] = lv_btn_create(container1, nullptr);
      lv_obj_set_style_local_bg_opa(itemApps[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
      lv_obj_set_style_local_radius(itemApps[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 57);
      lv_obj_set_style_local_bg_color(itemApps[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

      lv_obj_set_width(itemApps[i], LV_HOR_RES - 8);
      lv_obj_set_height(itemApps[i], 57);
      lv_obj_set_event_cb(itemApps[i], ButtonEventHandler);
      lv_btn_set_layout(itemApps[i], LV_LAYOUT_ROW_MID);
      itemApps[i]->user_data = this;

      labelBtIco = lv_label_create(itemApps[i], nullptr);
      lv_obj_set_style_local_text_color(labelBtIco, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
      lv_label_set_text_static(labelBtIco, applications[i].icon);

      labelBt = lv_label_create(itemApps[i], nullptr);
      lv_label_set_text_fmt(labelBt, " %s", applications[i].name);
    }
  }

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");
}

List::~List() {
  lv_obj_clean(lv_scr_act());
}

bool List::Refresh() {

  return running;
}

void List::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_RELEASED) {
    for (int i = 0; i < MAXLISTITEMS; i++) {
      if (apps[i] != Apps::None && object == itemApps[i]) {
        app->StartApp(apps[i], DisplayApp::FullRefreshDirections::Up);
        running = false;
        return;
      }
    }
  }
}
