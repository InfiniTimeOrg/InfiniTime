#include "displayapp/screens/CheckboxList.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Styles.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void event_handler(lv_obj_t* obj, lv_event_t event) {
    CheckboxList* screen = static_cast<CheckboxList*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }

}

CheckboxList::CheckboxList(const uint8_t screenID,
                           const uint8_t numScreens,
                           DisplayApp* app,
                           Controllers::Settings& settingsController,
                           const char* optionsTitle,
                           const char* optionsSymbol,
                           void (Controllers::Settings::*SetOptionIndex)(uint8_t),
                           uint8_t (Controllers::Settings::*GetOptionIndex )() const,
                           std::array<const char*, MAXLISTITEMS> options)
  : Screen(app), screenID {screenID}, settingsController {settingsController},
    SetOptionIndex {SetOptionIndex}, GetOptionIndex {GetOptionIndex},
    options {options} {

  settingsController.SetWatchfacesMenu(screenID);

  // Set the background to Black
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

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
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_set_pos(container1, 10, 60);
  lv_obj_set_width(container1, LV_HOR_RES - 20);
  lv_obj_set_height(container1, LV_VER_RES - 50);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, optionsTitle);
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, optionsSymbol);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  for (unsigned int i = 0; i < options.size(); i++) {
    if (strcmp(options[i], "")) {
      cbOption[i] = lv_checkbox_create(container1, nullptr);
      lv_checkbox_set_text(cbOption[i], options[i]);
      cbOption[i]->user_data = this;
      lv_obj_set_event_cb(cbOption[i], event_handler);
      SetRadioButtonStyle(cbOption[i]);

      if (static_cast<unsigned int>((settingsController.*GetOptionIndex)() - MAXLISTITEMS*screenID) == i) {
        lv_checkbox_set_checked(cbOption[i], true);
      }
    }
  }
}

CheckboxList::~CheckboxList() {
  lv_obj_clean(lv_scr_act());
  settingsController.SaveSettings();
}

void CheckboxList::UpdateSelected(lv_obj_t* object, lv_event_t event) {
  if (event == LV_EVENT_VALUE_CHANGED) {
    for (unsigned int i = 0; i < options.size(); i++) {
      if (strcmp(options[i], "")) {
        if (object == cbOption[i]) {
          lv_checkbox_set_checked(cbOption[i], true);
          (settingsController.*SetOptionIndex)(MAXLISTITEMS*screenID + i);
        } else {
          lv_checkbox_set_checked(cbOption[i], false);
        }
      }
    }
  }
}