#include "displayapp/screens/Metronome.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void eventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<Metronome*>(obj->user_data);
    screen->OnEvent(obj, event);
  }

  lv_obj_t* createLabel(const char* name, lv_obj_t* reference, lv_align_t align, lv_font_t* font, uint8_t x, uint8_t y) {
    lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    lv_label_set_text(label, name);
    lv_obj_align(label, reference, align, x, y);

    return label;
  }
}

Metronome::Metronome(Controllers::MotorController& motorController, System::SystemTask& systemTask)
  : motorController {motorController}, systemTask {systemTask} {

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

  bpmValue = createLabel("120", bpmArc, LV_ALIGN_IN_TOP_MID, &jetbrains_mono_76, 0, 55);
  createLabel("bpm", bpmValue, LV_ALIGN_OUT_BOTTOM_MID, &jetbrains_mono_bold_20, 0, 0);

  bpmTap = lv_btn_create(lv_scr_act(), nullptr);
  bpmTap->user_data = this;
  lv_obj_set_event_cb(bpmTap, eventHandler);
  lv_obj_set_style_local_bg_opa(bpmTap, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_height(bpmTap, 80);
  lv_obj_align(bpmTap, bpmValue, LV_ALIGN_IN_TOP_MID, 0, 0);

  bpbDropdown = lv_dropdown_create(lv_scr_act(), nullptr);
  bpbDropdown->user_data = this;
  lv_obj_set_event_cb(bpbDropdown, eventHandler);
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
  lblPlayPause = lv_label_create(playPause, nullptr);
  lv_label_set_text_static(lblPlayPause, Symbols::play);

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
          lv_label_set_text_static(lblPlayPause, Symbols::pause);
          systemTask.PushMessage(System::Messages::DisableSleeping);
          startTime = xTaskGetTickCount();
          counter = 1;
        } else {
          lv_label_set_text_static(lblPlayPause, Symbols::play);
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
  }
  return true;
}
