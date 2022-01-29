#include "displayapp/screens/Metronome.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void eventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Metronome*>(obj->user_data);
    screen->OnEvent(obj, event);
  }
}

Metronome::Metronome(DisplayApp* app, Controllers::MotorController& motorController, System::SystemTask& systemTask)
  : Screen(app), motorController {motorController}, systemTask {systemTask} {

  bpmArc = lv_arc_create(lv_scr_act(), nullptr);
  bpmArc->user_data = this;
  lv_obj_set_event_cb(bpmArc, eventHandler);
  lv_arc_set_bg_angles(bpmArc, 0, 270);
  lv_arc_set_rotation(bpmArc, 135);
  lv_arc_set_range(bpmArc, 40, 220);
  lv_arc_set_value(bpmArc, bpm);
  lv_obj_set_size(bpmArc, 210, 210);
  lv_arc_set_adjustable(bpmArc, true);
  lv_obj_align(bpmArc, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  bpmValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(bpmValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_label_set_text_static(bpmValue, "120");
  lv_obj_set_style_local_text_color(bpmValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_SILVER);
  lv_obj_align(bpmValue, bpmArc, LV_ALIGN_IN_TOP_MID, 0, 55);

  lv_obj_t* labelBpm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelBpm, "bpm");
  lv_obj_set_style_local_text_color(labelBpm, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_SILVER);
  lv_obj_align(labelBpm, bpmValue, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  bpmTap = lv_btn_create(lv_scr_act(), nullptr);
  bpmTap->user_data = this;
  lv_obj_set_event_cb(bpmTap, eventHandler);
  lv_obj_set_style_local_bg_opa(bpmTap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_height(bpmTap, 80);
  lv_obj_align(bpmTap, bpmValue, LV_ALIGN_IN_TOP_MID, 0, 0);

  bpbDropdown = lv_dropdown_create(lv_scr_act(), nullptr);
  bpbDropdown->user_data = this;
  lv_obj_set_event_cb(bpbDropdown, eventHandler);
  lv_obj_set_style_local_pad_left(bpbDropdown, LV_DROPDOWN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_pad_left(bpbDropdown, LV_DROPDOWN_PART_LIST, LV_STATE_DEFAULT, 20);
  lv_obj_set_size(bpbDropdown, 115, 50);
  lv_obj_align(bpbDropdown, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_dropdown_set_options(bpbDropdown, "1\n2\n3\n4\n5\n6\n7\n8\n9");
  lv_dropdown_set_selected(bpbDropdown, bpb - 1);
  lv_dropdown_set_show_selected(bpbDropdown, false);
  lv_dropdown_set_text(bpbDropdown, "");

  currentBpbText = lv_label_create(bpbDropdown, nullptr);
  lv_label_set_text_fmt(currentBpbText, "%d bpb", bpb);
  lv_obj_align(currentBpbText, bpbDropdown, LV_ALIGN_CENTER, 0, 0);

  playPause = lv_btn_create(lv_scr_act(), nullptr);
  playPause->user_data = this;
  lv_obj_set_event_cb(playPause, eventHandler);
  lv_obj_set_size(playPause, 115, 50);
  lv_obj_align(playPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_local_value_str(playPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::play);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Metronome::~Metronome() {
  lv_task_del(taskRefresh);
  systemTask.PushMessage(System::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

void Metronome::Refresh() {
  if (metronomeStarted) {
    if (xTaskGetTickCount() - startTime > 60u * configTICK_RATE_HZ / static_cast<uint16_t>(bpm)) {
      startTime += 60 * configTICK_RATE_HZ / bpm;
      counter--;
      if (counter == 0) {
        counter = bpb;
        motorController.RunForDuration(90);
      } else {
        motorController.RunForDuration(30);
      }
    }
  }
}

void Metronome::OnEvent(lv_obj_t* obj, lv_event_t event) {
  switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
      if (obj == bpmArc) {
        bpm = lv_arc_get_value(bpmArc);
        lv_label_set_text_fmt(bpmValue, "%03d", bpm);
      } else if (obj == bpbDropdown) {
        bpb = lv_dropdown_get_selected(obj) + 1;
        lv_label_set_text_fmt(currentBpbText, "%d bpb", bpb);
        lv_obj_realign(currentBpbText);
      }
      break;
    }
    case LV_EVENT_PRESSED: {
      if (obj == bpmTap) {
        TickType_t delta = xTaskGetTickCount() - tappedTime;
        if (tappedTime != 0 && delta < configTICK_RATE_HZ * 3) {
          bpm = configTICK_RATE_HZ * 60 / delta;
          lv_arc_set_value(bpmArc, bpm);
          lv_label_set_text_fmt(bpmValue, "%03d", bpm);
        }
        tappedTime = xTaskGetTickCount();
        allowExit = true;
      }
      break;
    }
    case LV_EVENT_RELEASED:
    case LV_EVENT_PRESS_LOST:
      if (obj == bpmTap) {
        allowExit = false;
      }
      break;
    case LV_EVENT_CLICKED: {
      if (obj == playPause) {
        metronomeStarted = !metronomeStarted;
        if (metronomeStarted) {
          lv_obj_set_style_local_value_str(playPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::pause);
          systemTask.PushMessage(System::Messages::DisableSleeping);
          startTime = xTaskGetTickCount();
          counter = 1;
        } else {
          lv_obj_set_style_local_value_str(playPause, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Symbols::play);
          systemTask.PushMessage(System::Messages::EnableSleeping);
        }
      }
      break;
    }
    default:
      break;
  }
}

bool Metronome::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::SwipeDown && allowExit) {
    running = false;
    return true;
  }
  return false;
}
