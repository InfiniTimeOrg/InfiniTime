#include "displayapp/screens/FirmwareValidation.h"
#include <lvgl/lvgl.h>
#include "Version.h"
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/localization/Localization.h"

using namespace Pinetime::Applications::Screens;
using namespace Pinetime::Applications::Localization;

namespace {
  void ButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<FirmwareValidation*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }
}

FirmwareValidation::FirmwareValidation(Pinetime::Controllers::FirmwareValidator& validator,
                                       Pinetime::Controllers::Settings& settingsController)
  : validator {validator}, settingsController {settingsController} {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, Translate(settingsController.GetLanguage(), StringId::Firmware));
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 10, 15);

  lv_obj_t* icon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_ORANGE);
  lv_label_set_text_static(icon, Symbols::check);
  lv_label_set_align(icon, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(icon, title, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  labelVersion = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(labelVersion, true);
  lv_label_set_text_fmt(labelVersion,
                        "#808080 %s# %lu.%lu.%lu\n"
                        "#808080 %s# %s\n",
                        Translate(settingsController.GetLanguage(), StringId::Version),
                        Version::Major(),
                        Version::Minor(),
                        Version::Patch(),
                        Translate(settingsController.GetLanguage(), StringId::ShortRef),
                        Version::GitCommitHash());
  lv_obj_align(labelVersion, nullptr, LV_ALIGN_CENTER, 0, -40);
  lv_label_set_align(labelVersion, LV_LABEL_ALIGN_CENTER);

  labelIsValidated = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelIsValidated, nullptr, LV_ALIGN_CENTER, 0, 10);
  lv_label_set_align(labelIsValidated, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_auto_realign(labelIsValidated, true);
  lv_label_set_recolor(labelIsValidated, true);

  if (validator.IsValidated()) {
    lv_label_set_text_static(labelIsValidated, Translate(settingsController.GetLanguage(), StringId::FirmwareValidated));
    lv_obj_align(labelIsValidated, nullptr, LV_ALIGN_CENTER, 0, 10);
  } else {
    lv_label_set_text_static(labelIsValidated, Translate(settingsController.GetLanguage(), StringId::FirmwareRollbackWarning));

    buttonValidate = lv_btn_create(lv_scr_act(), nullptr);
    buttonValidate->user_data = this;
    lv_obj_set_size(buttonValidate, 115, 50);
    lv_obj_align(buttonValidate, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_event_cb(buttonValidate, ButtonEventHandler);
    lv_obj_set_style_local_bg_color(buttonValidate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);

    labelButtonValidate = lv_label_create(buttonValidate, nullptr);
    lv_label_set_text_static(labelButtonValidate, Translate(settingsController.GetLanguage(), StringId::Validate));

    buttonReset = lv_btn_create(lv_scr_act(), nullptr);
    buttonReset->user_data = this;
    lv_obj_set_size(buttonReset, 115, 50);
    lv_obj_align(buttonReset, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_local_bg_color(buttonReset, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
    lv_obj_set_event_cb(buttonReset, ButtonEventHandler);

    labelButtonReset = lv_label_create(buttonReset, nullptr);
    lv_label_set_text_static(labelButtonReset, Translate(settingsController.GetLanguage(), StringId::Rollback));
  }
}

FirmwareValidation::~FirmwareValidation() {
  lv_obj_clean(lv_scr_act());
}

void FirmwareValidation::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (object == buttonValidate && event == LV_EVENT_CLICKED) {
    validator.Validate();
    running = false;
  } else if (object == buttonReset && event == LV_EVENT_CLICKED) {
    validator.Reset();
  }
}
