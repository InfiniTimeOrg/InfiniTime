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
  TickType_t calculateDelta(const TickType_t startTime, const TickType_t currentTime) {
    TickType_t delta = 0;
    // Take care of overflow
    if (startTime > currentTime) {
      delta = 0xffffffff - startTime;
      delta += (currentTime + 1);
    } else {
      delta = currentTime - startTime;
    }
    return delta;
  }
}

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  Metronome* screen = static_cast<Metronome*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

Metronome::Metronome(DisplayApp* app,
                     Controllers::MotorController& motorController,
                     System::SystemTask& systemTask)
  : Screen(app),
    running {true},
    currentState {States::Stopped},
    startTime {},
    motorController {motorController},
    systemTask {systemTask} {

  bpmText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(bpmText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
  lv_obj_set_style_local_text_color(bpmText, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text(bpmText, std::to_string(bpm).c_str());
  lv_obj_align(bpmText, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 0, -45);

  bpmLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(bpmLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
  lv_obj_set_style_local_text_color(bpmLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_label_set_text(bpmLabel, "bpm");
  lv_obj_align(bpmLabel, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 70, 3);

  bpmUp = lv_btn_create(lv_scr_act(), nullptr);
  bpmUp->user_data = this;
  lv_obj_set_event_cb(bpmUp, btnEventHandler);
  lv_obj_align(bpmUp, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 10, -70);
  lv_obj_set_height(bpmUp, 40);
  lv_obj_set_width(bpmUp, 60);
  bpmUpTxt = lv_label_create(bpmUp, nullptr);
  lv_label_set_text(bpmUpTxt, "+");

  bpmDown = lv_btn_create(lv_scr_act(), nullptr);
  bpmDown->user_data = this;
  lv_obj_set_event_cb(bpmDown, btnEventHandler);
  lv_obj_align(bpmDown, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 10, -20);
  lv_obj_set_height(bpmDown, 40);
  lv_obj_set_width(bpmDown, 60);
  bpmDownTxt = lv_label_create(bpmDown, nullptr);
  lv_label_set_text(bpmDownTxt, "-");

  btnPlayPause = lv_btn_create(lv_scr_act(), nullptr);
  btnPlayPause->user_data = this;
  lv_obj_set_event_cb(btnPlayPause, btnEventHandler);
  lv_obj_align(btnPlayPause, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -10);
  lv_obj_set_height(btnPlayPause, 40);
  txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  lv_label_set_text(txtPlayPause, Symbols::play);
}

Metronome::~Metronome() {
  lv_obj_clean(lv_scr_act());
}

bool Metronome::Refresh() {
  switch (currentState) {
    case States::Stopped: {
      break;
    }
    case States::Running: {
      TickType_t elapsedTime = calculateDelta(startTime, xTaskGetTickCount());

      if (static_cast<float>(elapsedTime) / static_cast<float>(configTICK_RATE_HZ) >= (60.0 / bpm)) {
        motorController.SetDuration(30);
        startTime = xTaskGetTickCount();
      }
      break;
    }
  }
  return running;
}

void Metronome::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnPlayPause) {
      currentState = (currentState == States::Stopped ? States::Running : States::Stopped);
      switch (currentState) {
        case States::Stopped: {
          lv_label_set_text(txtPlayPause, Symbols::play);
          systemTask.PushMessage(Pinetime::System::SystemTask::Messages::EnableSleeping);
          break;
        }
        case States::Running: {
          lv_label_set_text(txtPlayPause, Symbols::pause);
          systemTask.PushMessage(Pinetime::System::SystemTask::Messages::DisableSleeping);
          startTime = xTaskGetTickCount();
          break;
        }
      }
    } else {
      if (obj == bpmUp) {
        if (bpm >= 300) {
          bpm = 0;
        } else {
          bpm++;
        }
        lv_label_set_text_fmt(bpmText, "%03d", bpm);

      } else if (obj == bpmDown) {
        if (bpm == 0) {
          bpm = 300;
        } else {
          bpm--;
        }
        lv_label_set_text_fmt(bpmText, "%03d", bpm);
      }
    }
  }
}
