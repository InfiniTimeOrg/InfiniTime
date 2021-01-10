#include <libs/lvgl/lvgl.h>
#include "HeartRate.h"
#include <components/heartrate/HeartRateController.h>

#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;

const char* ToString(Pinetime::Controllers::HeartRateController::States s) {
  switch(s) {
    case Pinetime::Controllers::HeartRateController::States::NotEnoughData: return "Not enough data,\nplease wait...";
    case Pinetime::Controllers::HeartRateController::States::NoTouch: return "No touch detected";
    case Pinetime::Controllers::HeartRateController::States::Running: return "Measuring...";
  }
  return "";
}

HeartRate::HeartRate(Pinetime::Applications::DisplayApp *app, Controllers::HeartRateController& heartRateController) : Screen(app), heartRateController{heartRateController} {
  label_bpm = lv_label_create(lv_scr_act(), NULL);

  labelStyle = const_cast<lv_style_t *>(lv_label_get_style(label_bpm, LV_LABEL_STYLE_MAIN));
  labelStyle->text.font = &jetbrains_mono_bold_20;

  lv_style_copy(&labelBigStyle, labelStyle);
  labelBigStyle.text.font = &jetbrains_mono_extrabold_compressed;

  lv_label_set_style(label_bpm, LV_LABEL_STYLE_MAIN, labelStyle);

  label_hr = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_style(label_hr, LV_LABEL_STYLE_MAIN, &labelBigStyle);
  lv_obj_align(label_hr, lv_scr_act(), LV_ALIGN_CENTER, -70, 0);
  lv_label_set_text(label_hr, "000");

  lv_label_set_text(label_bpm, "Heart rate BPM");
  lv_obj_align(label_bpm, label_hr, LV_ALIGN_OUT_TOP_MID, 0, -20);


  label_status = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label_status, ToString(Pinetime::Controllers::HeartRateController::States::NotEnoughData));
  lv_label_set_style(label_status, LV_LABEL_STYLE_MAIN, labelStyle);
  lv_obj_align(label_status, label_hr, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  heartRateController.Start();
}

HeartRate::~HeartRate() {
  heartRateController.Stop();
  lv_obj_clean(lv_scr_act());
}

bool HeartRate::Refresh() {
  char hr[4];

  auto state = heartRateController.State();
  switch(state) {
    case Controllers::HeartRateController::States::NoTouch:
    case Controllers::HeartRateController::States::NotEnoughData:
      lv_label_set_text(label_hr, "000");
      break;
    default:
      sprintf(hr, "%03d", heartRateController.HeartRate());
      lv_label_set_text(label_hr, hr);
  }

  lv_label_set_text(label_status, ToString(state));
  lv_obj_align(label_status, label_hr, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

  return running;
}

bool HeartRate::OnButtonPushed() {
  running = false;
  return true;
}
