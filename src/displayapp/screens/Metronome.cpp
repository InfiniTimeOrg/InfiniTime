#include "Metronome.h"

#include "Screen.h"
#include "Symbols.h"
#include "lvgl/lvgl.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include <string>
#include <tuple>

using namespace Pinetime::Applications::Screens;

namespace {
  float calculateDelta(const TickType_t startTime, const TickType_t currentTime) {
    TickType_t delta = 0;
    // Take care of overflow
    if (startTime > currentTime) {
      delta = 0xffffffff - startTime;
      delta += (currentTime + 1);
    } else {
      delta = currentTime - startTime;
    }
    return static_cast<float>(delta) / static_cast<float>(configTICK_RATE_HZ);
  }

  static void eventHandler(lv_obj_t* obj, lv_event_t event) {
    Metronome* screen = static_cast<Metronome*>(obj->user_data);
    screen->OnEvent(obj, event);
  }

  lv_obj_t* createLabel(const char* name, lv_obj_t* reference, lv_align_t align, lv_font_t* font, uint8_t x = 0, uint8_t y = 0) {
    lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_label_set_text(label, name);
    lv_obj_align(label, reference, align, x, y);

    return label;
  }
}

Metronome::Metronome(DisplayApp* app, Controllers::MotorController& motorController, System::SystemTask& systemTask)
  : Screen(app), running {true}, currentState {States::Stopped}, startTime {}, motorController {motorController}, systemTask {systemTask} {

  bpmArc = lv_arc_create(lv_scr_act(), nullptr);
  bpmArc->user_data = this;
  lv_obj_set_event_cb(bpmArc, eventHandler);
  lv_arc_set_bg_angles(bpmArc, 0, 270);
  lv_arc_set_rotation(bpmArc, 135);
  lv_arc_set_range(bpmArc, 40, 220);
  lv_arc_set_value(bpmArc, bpm);
  lv_obj_set_size(bpmArc, 210, 210);
  lv_arc_set_adjustable(bpmArc, true);
  lv_obj_align(bpmArc, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 7);

  bpmValue = createLabel(std::to_string(lv_arc_get_value(bpmArc)).c_str(), bpmArc, LV_ALIGN_IN_TOP_MID, &jetbrains_mono_76, 0, 55);
  bpmLegend = createLabel("bpm", bpmValue, LV_ALIGN_OUT_BOTTOM_MID, &jetbrains_mono_bold_20, 0, 0);

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
  lv_obj_align(bpbDropdown, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 15, -4);
  lv_dropdown_set_options(bpbDropdown, "1\n2\n3\n4\n5\n6\n7\n8\n9");
  lv_dropdown_set_selected(bpbDropdown, bpb - 1);
  bpbLegend = lv_label_create(bpbDropdown, nullptr);
  lv_label_set_text(bpbLegend, "bpb");
  lv_obj_align(bpbLegend, bpbDropdown, LV_ALIGN_IN_RIGHT_MID, -15, 0);

  playPause = lv_btn_create(lv_scr_act(), nullptr);
  playPause->user_data = this;
  lv_obj_set_event_cb(playPause, eventHandler);
  lv_obj_align(playPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -15, -10);
  lv_obj_set_height(playPause, 39);
  playPauseLabel = lv_label_create(playPause, nullptr);
  lv_label_set_text(playPauseLabel, Symbols::play);

  app->SetTouchMode(DisplayApp::TouchModes::Polling);
}

Metronome::~Metronome() {
  app->SetTouchMode(DisplayApp::TouchModes::Gestures);
  systemTask.PushMessage(Pinetime::System::SystemTask::Messages::EnableSleeping);
  lv_obj_clean(lv_scr_act());
}

bool Metronome::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return true;
}

bool Metronome::Refresh() {
  switch (currentState) {
    case States::Stopped: {
      break;
    }
    case States::Running: {
      if (calculateDelta(startTime, xTaskGetTickCount()) >= (60.0 / bpm)) {
        counter--;
        startTime -= 60.0 / bpm;
        startTime = xTaskGetTickCount();
        if (counter == 0) {
          counter = bpb;
          motorController.SetDuration(90);
        } else {
          motorController.SetDuration(30);
        }
      }
      break;
    }
  }
  return running;
}

void Metronome::OnEvent(lv_obj_t* obj, lv_event_t event) {
  switch (event) {
    case LV_EVENT_VALUE_CHANGED: {
      if (obj == bpmArc) {
        bpm = lv_arc_get_value(bpmArc);
        lv_label_set_text_fmt(bpmValue, "%03d", bpm);
      } else if (obj == bpbDropdown) {
        bpb = lv_dropdown_get_selected(obj) + 1;
      }
      break;
    }
    case LV_EVENT_PRESSED: {
      if (obj == bpmTap) {
        float timeDelta = calculateDelta(tappedTime, xTaskGetTickCount());
        if (tappedTime == 0 || timeDelta > 3) {
          tappedTime = xTaskGetTickCount();
        } else {
          bpm = ceil(60.0 / timeDelta);
          lv_arc_set_value(bpmArc, bpm);
          lv_label_set_text_fmt(bpmValue, "%03d", bpm);
          tappedTime = xTaskGetTickCount();
        }
      }
      break;
    }
    case LV_EVENT_CLICKED: {
      if (obj == playPause) {
        currentState = (currentState == States::Stopped ? States::Running : States::Stopped);
        switch (currentState) {
          case States::Stopped: {
            lv_label_set_text(playPauseLabel, Symbols::play);
            systemTask.PushMessage(Pinetime::System::SystemTask::Messages::EnableSleeping);
            break;
          }
          case States::Running: {
            lv_label_set_text(playPauseLabel, Symbols::pause);
            systemTask.PushMessage(Pinetime::System::SystemTask::Messages::DisableSleeping);
            startTime = xTaskGetTickCount();
            counter = 1;
            break;
          }
        }
      }
      break;
    }
  }
}
