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
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

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
  lv_obj_set_style_pad_all(container1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_gap(container1, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(container1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_pos(container1, 0, 0);
  lv_obj_set_width(container1, LV_HOR_RES - 2);
  lv_obj_set_height(container1, LV_VER_RES);
  lv_obj_set_layout(container1, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(container1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(container1, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t* labelBt;
  lv_obj_t* labelBtIco;

  for (int i = 0; i < MAXLISTITEMS; i++) {
    apps[i] = applications[i].application;
    
    itemApps[i] = lv_btn_create(container1);
    itemApps[i]->user_data = this;
    lv_obj_set_flex_grow(itemApps[i], 1);
    lv_obj_set_width(itemApps[i], lv_pct(100));
  
    if (applications[i].application == Apps::None){
      lv_obj_set_style_bg_opa(itemApps[i], LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    } else{
      lv_obj_set_style_bg_opa(itemApps[i], LV_OPA_20, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_radius(itemApps[i], LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_bg_color(itemApps[i], lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

      lv_obj_add_event_cb(itemApps[i], ButtonEventHandler, LV_EVENT_ALL, itemApps[i]->user_data);
      
      labelBt = lv_label_create(itemApps[i]);
      lv_label_set_recolor(labelBt, true);
      lv_label_set_text_fmt(labelBt, "#FFEB3B %s #%s", applications[i].icon, applications[i].name);
      lv_obj_align(labelBt, LV_ALIGN_LEFT_MID, 10, 0);
    }
  }
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
