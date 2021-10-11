#include "List.h"
#include "../DisplayApp.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void ButtonEventHandler(lv_event_t* event) {
    List* screen = static_cast<List*>(lv_event_get_user_data(event));
    screen->OnButtonEvent(lv_event_get_target(event), event);
  }

}

List::List(uint8_t screenID,
           uint8_t numScreens,
           DisplayApp* app,
           Controllers::Settings& settingsController,
           std::array<Applications, MAXLISTITEMS>& applications)
  : Screen(app), settingsController {settingsController} {

  // Set the background to Black
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(0, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);

  settingsController.SetSettingsMenu(screenID);

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

  lv_obj_t* container1 = lv_obj_create(lv_scr_act());

  lv_obj_set_style_bg_opa(container1, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_gap(container1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_pos(container1, 0, 0);
  lv_obj_set_width(container1, LV_HOR_RES - 8);
  lv_obj_set_height(container1, LV_VER_RES);
  lv_obj_set_layout(container1, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START);

  lv_obj_t* labelBt;
  lv_obj_t* labelBtIco;

  for (int i = 0; i < MAXLISTITEMS; i++) {
    apps[i] = applications[i].application;
    if (applications[i].application != Apps::None) {

      itemApps[i] = lv_btn_create(container1);
      itemApps[i]->user_data = this;
      lv_obj_set_style_bg_opa(itemApps[i], LV_OPA_20, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_radius(itemApps[i], 57, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(itemApps[i], lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

      lv_obj_set_width(itemApps[i], LV_HOR_RES - 8);
      lv_obj_set_height(itemApps[i], 57);
      lv_obj_add_event_cb(itemApps[i], ButtonEventHandler, LV_EVENT_ALL, itemApps[i]->user_data);
      lv_obj_set_layout(itemApps[i], LV_LAYOUT_FLEX);
      lv_obj_set_flex_flow(itemApps[i], LV_FLEX_FLOW_ROW);
      lv_obj_set_flex_align(itemApps[i], LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

      labelBtIco = lv_label_create(itemApps[i]);
      lv_obj_set_style_text_color(labelBtIco, lv_palette_main(LV_PALETTE_YELLOW), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_label_set_text_static(labelBtIco, applications[i].icon);

      labelBt = lv_label_create(itemApps[i]);
      lv_label_set_text_fmt(labelBt, " %s", applications[i].name);
    }
  }

  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CLIP);
  lv_obj_set_size(backgroundLabel, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");
}

List::~List() {
  lv_obj_clean(lv_scr_act());
}

void List::OnButtonEvent(lv_obj_t* object, lv_event_t* event) {
  if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
    for (int i = 0; i < MAXLISTITEMS; i++) {
      if (apps[i] != Apps::None && object == itemApps[i]) {
        app->StartApp(apps[i], DisplayApp::FullRefreshDirections::Up);
        running = false;
        return;
      }
    }
  }
}
