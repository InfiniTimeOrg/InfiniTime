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
  labelVersion = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(labelVersion,
                        "Version : %d.%d.%d\n"
                        "ShortRef : %s",
                        Version::Major(),
                        Version::Minor(),
                        Version::Patch(),
                        Version::GitCommitHash());
  lv_obj_align(labelVersion, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  labelIsValidated = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelIsValidated, labelVersion, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
  lv_label_set_recolor(labelIsValidated, true);
  lv_label_set_long_mode(labelIsValidated, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(labelIsValidated, 240);

  if (validator.IsValidated())
    lv_label_set_text(labelIsValidated, "You have already\n#00ff00 validated# this firmware#");
  else {
    lv_label_set_text(labelIsValidated, "Please #00ff00 Validate# this version or\n#ff0000 Reset# to rollback to the previous version.");

    buttonValidate = lv_btn_create(lv_scr_act(), nullptr);
    buttonValidate->user_data = this;
    lv_obj_set_size(buttonValidate, 115, 50);
    lv_obj_align(buttonValidate, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_event_cb(buttonValidate, ButtonEventHandler);
    lv_obj_set_style_local_bg_color(buttonValidate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x009900));

    labelButtonValidate = lv_label_create(buttonValidate, nullptr);
    lv_label_set_text_static(labelButtonValidate, "Validate");

    buttonReset = lv_btn_create(lv_scr_act(), nullptr);
    buttonReset->user_data = this;
    lv_obj_set_size(buttonReset, 115, 50);
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
