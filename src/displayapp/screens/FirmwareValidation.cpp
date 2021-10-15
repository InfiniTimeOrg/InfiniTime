#include "FirmwareValidation.h"
#include <lvgl/lvgl.h>
#include "Version.h"
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void ButtonEventHandler(lv_event_t* event) {
    FirmwareValidation* screen = static_cast<FirmwareValidation*>(lv_event_get_user_data(event));
    screen->OnButtonEvent(lv_event_get_target(event), event);
  }

}

FirmwareValidation::FirmwareValidation(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::FirmwareValidator& validator)
  : Screen {app}, validator {validator} {
  labelVersion = lv_label_create(lv_scr_act());
  lv_label_set_text_fmt(labelVersion,
                        "Version : %lu.%lu.%lu\n"
                        "ShortRef : %s",
                        Version::Major(),
                        Version::Minor(),
                        Version::Patch(),
                        Version::GitCommitHash());
  lv_obj_align(labelVersion, LV_ALIGN_TOP_LEFT, 0, 0);

  labelIsValidated = lv_label_create(lv_scr_act());
  lv_obj_align_to(labelIsValidated, labelVersion, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
  lv_label_set_recolor(labelIsValidated, true);
  lv_label_set_long_mode(labelIsValidated, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(labelIsValidated, 240);

  if (validator.IsValidated())
    lv_label_set_text(labelIsValidated, "You have already\n#00ff00 validated# this firmware#");
  else {
    lv_label_set_text(labelIsValidated, "Please #00ff00 Validate# this version or\n#ff0000 Reset# to rollback to the previous version.");

    buttonValidate = lv_btn_create(lv_scr_act());
    buttonValidate->user_data = this;
    lv_obj_set_size(buttonValidate, 115, 50);
    lv_obj_align(buttonValidate, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_event_cb(buttonValidate, ButtonEventHandler, LV_EVENT_ALL, buttonValidate->user_data);
    lv_obj_set_style_bg_color(buttonValidate, lv_color_hex(0x009900), LV_PART_MAIN | LV_STATE_DEFAULT);

    labelButtonValidate = lv_label_create(buttonValidate);
    lv_label_set_text_static(labelButtonValidate, "Validate");
    lv_obj_center(labelButtonValidate);

    buttonReset = lv_btn_create(lv_scr_act());
    buttonReset->user_data = this;
    lv_obj_set_size(buttonReset, 115, 50);
    lv_obj_align(buttonReset, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(buttonReset, lv_color_hex(0x990000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(buttonReset, ButtonEventHandler, LV_EVENT_ALL, buttonReset->user_data);

    labelButtonReset = lv_label_create(buttonReset);
    lv_label_set_text_static(labelButtonReset, "Reset");
    lv_obj_center(labelButtonReset);
  }
}

FirmwareValidation::~FirmwareValidation() {
  lv_obj_clean(lv_scr_act());
}

void FirmwareValidation::OnButtonEvent(lv_obj_t* object, lv_event_t* event) {
  if (object == buttonValidate && lv_event_get_code(event) == LV_EVENT_CLICKED) {
    validator.Validate();
    running = false;
  } else if (object == buttonReset && lv_event_get_code(event) == LV_EVENT_CLICKED) {
    validator.Reset();
  }
}
