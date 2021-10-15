#include "Label.h"

using namespace Pinetime::Applications::Screens;

Label::Label(uint8_t screenID, uint8_t numScreens, Pinetime::Applications::DisplayApp* app)
  : Label(screenID, numScreens, app, nullptr){};

Label::Label(uint8_t screenID, uint8_t numScreens, Pinetime::Applications::DisplayApp* app, lv_obj_t* container)
  : Screen(app), labelText {container} {
  
  if (labelText == nullptr){
    labelText = lv_obj_create(lv_scr_act());
    lv_obj_set_layout(labelText, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(labelText, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(labelText, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(labelText, LV_OPA_TRANSP, 0);
    lv_obj_set_size(labelText, LV_HOR_RES, LV_SIZE_CONTENT);
    lv_obj_center(labelText);
  }
  
  if (numScreens > 1) {
    pageIndicatorBasePoints[0].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[0].y = 0;
    pageIndicatorBasePoints[1].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[1].y = LV_VER_RES;

    pageIndicatorBase = lv_line_create(lv_scr_act());
    lv_obj_set_style_line_width(pageIndicatorBase, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(pageIndicatorBase, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(pageIndicatorBase, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

    uint16_t indicatorSize = LV_VER_RES / numScreens;
    uint16_t indicatorPos = indicatorSize * screenID;

    pageIndicatorPoints[0].x = LV_HOR_RES - 1;
    pageIndicatorPoints[0].y = indicatorPos;
    pageIndicatorPoints[1].x = LV_HOR_RES - 1;
    pageIndicatorPoints[1].y = indicatorPos + indicatorSize;

    pageIndicator = lv_line_create(lv_scr_act());
    lv_obj_set_style_line_width(pageIndicator, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(pageIndicator, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(pageIndicator, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
  }
}

Label::~Label() {
  lv_obj_clean(lv_scr_act());
}

lv_obj_t* Label::addLine(){
  lv_obj_t* label = lv_label_create(labelText);
  lv_label_set_text(label, "");
  return label;
}

LV_FORMAT_ATTRIBUTE(2, 3) lv_obj_t* Label::addLine(const char* fmt, ...){
  lv_obj_t* label = lv_label_create(labelText);
  lv_label_set_recolor(label, true);
  
  va_list args;
  va_start(args, fmt);
  (reinterpret_cast<lv_label_t*>(label))->text = _lv_txt_set_text_vfmt(fmt, args);
  va_end(args);
  return label;
}

LV_FORMAT_ATTRIBUTE(2, 3) lv_obj_t* Label::addLineCenter(const char* fmt, ...){
  lv_obj_t* label = lv_label_create(labelText);
  lv_label_set_recolor(label, true);
  
  va_list args;
  va_start(args, fmt);
  (reinterpret_cast<lv_label_t*>(label))->text = _lv_txt_set_text_vfmt(fmt, args);
  va_end(args);
  
  lv_obj_set_width(label, LV_PCT(100));
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  
  return label;
}
