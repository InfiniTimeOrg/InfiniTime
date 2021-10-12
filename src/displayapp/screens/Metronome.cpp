#include "Metronome.h"
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void eventHandler(lv_event_t* event) {
    auto* screen = static_cast<Metronome*>(lv_event_get_user_data(event));
    screen->OnEvent(lv_event_get_target(event), event);
  }

  lv_obj_t* createLabel(const char* name, lv_obj_t* reference, lv_align_t align, const lv_font_t* font, uint8_t x, uint8_t y) {
    lv_obj_t* label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(label, name);
    lv_obj_align_to(label, reference, align, x, y);

    return label;
  }
}

Metronome::Metronome(DisplayApp* app, Controllers::MotorController& motorController, System::SystemTask& systemTask)
  : Screen(app), motorController {motorController}, systemTask {systemTask} {

  bpmArc = lv_arc_create(lv_scr_act());
  bpmArc->user_data = this;
  lv_obj_add_event_cb(bpmArc, eventHandler, LV_EVENT_ALL, bpmArc->user_data);
  lv_arc_set_bg_angles(bpmArc, 0, 270);
  lv_arc_set_rotation(bpmArc, 135);
  lv_arc_set_range(bpmArc, 40, 220);
  lv_arc_set_value(bpmArc, bpm);
  lv_obj_set_size(bpmArc, 210, 210);
  lv_obj_add_flag(bpmArc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_align(bpmArc, LV_ALIGN_TOP_MID, 0, 0);

  bpmValue = createLabel("120", bpmArc, LV_ALIGN_TOP_MID, &jetbrains_mono_76, 0, 55);
  createLabel("bpm", bpmValue, LV_ALIGN_OUT_BOTTOM_MID, &jetbrains_mono_bold_20, 0, 0);

  bpmTap = lv_btn_create(lv_scr_act());
  bpmTap->user_data = this;
  lv_obj_add_event_cb(bpmTap, eventHandler, LV_EVENT_ALL, bpmTap->user_data);
  lv_obj_set_style_bg_opa(bpmTap, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_height(bpmTap, 80);
  lv_obj_align_to(bpmTap, bpmValue, LV_ALIGN_TOP_MID, 0, 0);

  bpbDropdown = lv_dropdown_create(lv_scr_act());
  bpbDropdown->user_data = this;
  lv_obj_add_event_cb(bpbDropdown, eventHandler, LV_EVENT_ALL, bpbDropdown->user_data);
  lv_obj_set_style_pad_left(bpbDropdown, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  // The styling for the list itself is set in the event handler, as the "list" object does not exist until it is openened.
  // See https://docs.lvgl.io/8.0/widgets/core/dropdown.html#parts-and-styles
  //  lv_obj_set_style_pad_left(bpbDropdown, 20, LV_DROPDOWN_PART_LIST | LV_STATE_DEFAULT);
  lv_obj_set_size(bpbDropdown, 115, 50);
  lv_obj_align(bpbDropdown, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_dropdown_set_options(bpbDropdown, "1\n2\n3\n4\n5\n6\n7\n8\n9");
  lv_dropdown_set_selected(bpbDropdown, bpb - 1);
  lv_dropdown_set_text(bpbDropdown, "");

  currentBpbText = lv_label_create(bpbDropdown);
  lv_label_set_text_fmt(currentBpbText, "%d bpb", bpb);
  lv_obj_align(currentBpbText, LV_ALIGN_CENTER, 0, 0);

  playPause = lv_btn_create(lv_scr_act());
  playPause->user_data = this;
  lv_obj_add_event_cb(playPause, eventHandler, LV_EVENT_ALL, playPause->user_data);
  lv_obj_set_size(playPause, 115, 50);
  lv_obj_align(playPause, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  txtPlayPause = createLabel(LV_SYMBOL_PLAY, playPause, LV_ALIGN_CENTER, LV_FONT_DEFAULT, 0, 0);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, this);
}

Metronome::~Metronome() {
  lv_timer_del(taskRefresh);
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

void Metronome::OnEvent(lv_obj_t* obj, lv_event_t* event) {
  switch (lv_event_get_code(event)) {
    case LV_EVENT_VALUE_CHANGED: {
      if (obj == bpmArc) {
        bpm = lv_arc_get_value(bpmArc);
        lv_label_set_text_fmt(bpmValue, "%03d", bpm);
      } else if (obj == bpbDropdown) {
        lv_obj_t *list = lv_dropdown_get_list(obj);
        if(list != NULL) { // The list is open, we can style it now
          lv_obj_set_style_pad_left(list, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        bpb = lv_dropdown_get_selected(obj) + 1;
        lv_label_set_text_fmt(currentBpbText, "%d bpb", bpb);
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
      }
      break;
    }
    case LV_EVENT_CLICKED: {
      if (obj == playPause) {
        metronomeStarted = !metronomeStarted;
        if (metronomeStarted) {
          lv_label_set_text(txtPlayPause, LV_SYMBOL_PAUSE);
          systemTask.PushMessage(System::Messages::DisableSleeping);
          startTime = xTaskGetTickCount();
          counter = 1;
        } else {
          lv_label_set_text(txtPlayPause, LV_SYMBOL_PLAY);
          systemTask.PushMessage(System::Messages::EnableSleeping);
        }
      }
      break;
    }
    default:
      break;
  }
}
