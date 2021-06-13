#include "FirmwareValidation.h"
#include <lvgl/lvgl.h>
#include "Version.h"
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void ButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    FirmwareValidation* screen = static_cast<FirmwareValidation*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }

}

FirmwareValidation::FirmwareValidation(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::FirmwareValidator& validator)
  : Screen {app}, validator {validator} {
  labelVersionInfo = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelVersionInfo, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_label_set_text(labelVersionInfo, "Version : ");
  lv_label_set_align(labelVersionInfo, LV_LABEL_ALIGN_LEFT);

  labelVersionValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelVersionValue, labelVersionInfo, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  lv_label_set_recolor(labelVersionValue, true);
  sprintf(version, "%ld.%ld.%ld", Version::Major(), Version::Minor(), Version::Patch());
  lv_label_set_text(labelVersionValue, version);

  labelShortRefInfo = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelShortRefInfo, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 25);
  lv_label_set_text(labelShortRefInfo, "ShortRef : ");
  lv_label_set_align(labelShortRefInfo, LV_LABEL_ALIGN_LEFT);

  labelShortRefValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelShortRefValue, labelShortRefInfo, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  lv_label_set_recolor(labelShortRefValue, true);
  sprintf(shortref, "%s", Version::GitCommitHash());
  lv_label_set_text(labelShortRefValue, shortref);

  labelIsValidated = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelIsValidated, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 50);
  lv_label_set_recolor(labelIsValidated, true);
  lv_label_set_long_mode(labelIsValidated, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(labelIsValidated, 240);

  if (validator.IsValidated())
    lv_label_set_text(labelIsValidated, "You have already\n#00ff00 validated# this firmware#");
  else {
    lv_label_set_text(labelIsValidated, "Please #00ff00 Validate# this version or\n#ff0000 Reset# to rollback to the previous version.");

    buttonValidate = lv_btn_create(lv_scr_act(), nullptr);
    buttonValidate->user_data = this;
    lv_obj_set_height(buttonValidate, 50);
    lv_obj_align(buttonValidate, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_event_cb(buttonValidate, ButtonEventHandler);
    lv_obj_set_style_local_bg_color(buttonValidate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x009900));

    labelButtonValidate = lv_label_create(buttonValidate, nullptr);
    lv_label_set_text_static(labelButtonValidate, "Validate");

    buttonReset = lv_btn_create(lv_scr_act(), nullptr);
    buttonReset->user_data = this;
    lv_obj_set_height(buttonReset, 50);
    lv_obj_align(buttonReset, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_local_bg_color(buttonReset, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x990000));
    lv_obj_set_event_cb(buttonReset, ButtonEventHandler);

    labelButtonReset = lv_label_create(buttonReset, nullptr);
    lv_label_set_text_static(labelButtonReset, "Reset");
  }
}

FirmwareValidation::~FirmwareValidation() {
  lv_obj_clean(lv_scr_act());
}

bool FirmwareValidation::Refresh() {
  return running;
}

void FirmwareValidation::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (object == buttonValidate && event == LV_EVENT_PRESSED) {
    validator.Validate();
    running = false;
  } else if (object == buttonReset && event == LV_EVENT_PRESSED) {
    validator.Reset();
  }
}
