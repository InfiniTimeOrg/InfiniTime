#include "displayapp/screens/Trackers.h"

#include <algorithm>
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;
using Tracker = Pinetime::Controllers::TrackerDetector::Tracker;
using Detector = Pinetime::Controllers::TrackerDetector;

namespace {
  void ButtonEventHandler(lv_obj_t* object, lv_event_t event) {
    auto* screen = static_cast<Trackers*>(object->user_data);
    screen->OnButtonEvent(object, event);
  }

  // Signal strength as 0-5 bars
  uint8_t Bars(int8_t rssi) {
    if (rssi >= -55) {
      return 5;
    }
    if (rssi >= -65) {
      return 4;
    }
    if (rssi >= -75) {
      return 3;
    }
    if (rssi >= -85) {
      return 2;
    }
    return 1;
  }
}

Trackers::Trackers(Controllers::NimbleController& nimble) : nimble {nimble} {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::neonMagenta);
  lv_label_set_text_static(title, "// TRACKERS");
  lv_obj_set_pos(title, 4, 2);

  labelScanning = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelScanning, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::neonCyan);
  lv_label_set_text_static(labelScanning, "SCAN");
  lv_obj_align(labelScanning, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 2);

  buttonToggle = lv_btn_create(lv_scr_act(), nullptr);
  buttonToggle->user_data = this;
  lv_obj_set_event_cb(buttonToggle, ButtonEventHandler);
  lv_obj_set_size(buttonToggle, 236, 40);
  lv_obj_set_pos(buttonToggle, 2, 30);
  labelToggle = lv_label_create(buttonToggle, nullptr);

  labelEmpty = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelEmpty, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(labelEmpty, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(labelEmpty, 232);
  lv_label_set_text_static(labelEmpty,
                           "No trackers nearby.\n\n"
                           "Scanning live while\n"
                           "this is open. Auto\n"
                           "scan checks every\n"
                           "5 minutes.");
  lv_obj_set_pos(labelEmpty, 4, 84);

  labelPage = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelPage, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::neonPurple);

  for (uint8_t i = 0; i < perPage; i++) {
    const lv_coord_t y = 80 + i * 52;
    labelNames[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_recolor(labelNames[i], true);
    lv_obj_set_pos(labelNames[i], 4, y);
    labelDetails[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_recolor(labelDetails[i], true);
    lv_obj_set_pos(labelDetails[i], 4, y + 22);
  }

  UpdateToggle();
  nimble.StartTrackerScan(true);
  Refresh();
  taskRefresh = lv_task_create(RefreshTaskCallback, 1000, LV_TASK_PRIO_MID, this);
}

Trackers::~Trackers() {
  lv_task_del(taskRefresh);
  nimble.StopTrackerScan();
  lv_obj_clean(lv_scr_act());
}

void Trackers::UpdateToggle() {
  const bool enabled = nimble.IsTrackerScanEnabled();
  lv_label_set_text_static(labelToggle, enabled ? "AUTO SCAN ON" : "AUTO SCAN OFF");
  lv_obj_set_style_local_text_color(labelToggle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, enabled ? Colors::neonCyan : Colors::gray);
  lv_obj_set_style_local_border_color(buttonToggle, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, enabled ? Colors::neonCyan : Colors::gray);
}

void Trackers::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (object == buttonToggle && event == LV_EVENT_CLICKED) {
    nimble.SetTrackerScanEnabled(!nimble.IsTrackerScanEnabled());
    UpdateToggle();
  }
}

void Trackers::Refresh() {
  // Keep the live scan going while the app is on screen. It stops by itself shortly after the
  // screen turns off, since this task doesn't run then.
  if (!nimble.IsTrackerScanning() || ++ticks >= liveScanRenewSeconds) {
    ticks = 0;
    nimble.StartTrackerScan(true);
  }
  nimble.EvaluateTrackers();
  count = nimble.TrackerSnapshot(trackers);

  // Trackers that are following first, then the strongest signal (closest) first
  std::sort(trackers.begin(), trackers.begin() + count, [](const Tracker& a, const Tracker& b) {
    if (a.IsFollowing() != b.IsFollowing()) {
      return a.IsFollowing();
    }
    return a.rssi > b.rssi;
  });

  lv_obj_set_hidden(labelScanning, !nimble.IsTrackerScanning());
  ShowList();
}

void Trackers::ShowList() {
  const uint8_t pages = std::max<uint8_t>((count + perPage - 1) / perPage, 1);
  if (page >= pages) {
    page = pages - 1;
  }
  lv_obj_set_hidden(labelEmpty, count != 0);
  lv_obj_set_hidden(labelPage, pages < 2);
  lv_label_set_text_fmt(labelPage, "%d/%d", page + 1, pages);
  lv_obj_align(labelPage, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -4, -2);

  for (uint8_t i = 0; i < perPage; i++) {
    const size_t index = page * perPage + i;
    const bool visible = index < count;
    lv_obj_set_hidden(labelNames[i], !visible);
    lv_obj_set_hidden(labelDetails[i], !visible);
    if (!visible) {
      continue;
    }
    const Tracker& tracker = trackers[index];
    const bool following = tracker.IsFollowing();

    char bars[6] = "     ";
    for (uint8_t b = 0; b < Bars(tracker.rssi); b++) {
      bars[b] = '|';
    }
    lv_label_set_text_fmt(labelNames[i],
                          "%s %-12s#  #fcee0a %4d#",
                          following ? "#ff2a6d" : "#05d9e8",
                          Detector::Name(tracker.type),
                          tracker.rssi);
    lv_label_set_text_fmt(labelDetails[i],
                          "%s %3um %-6s#  #fcee0a %s#",
                          following ? "#ff2a6d" : "#808080",
                          static_cast<unsigned>(tracker.FollowingSeconds() / 60),
                          following ? "FOLLOW" : "nearby",
                          bars);
  }
}

bool Trackers::OnTouchEvent(TouchEvents event) {
  const uint8_t pages = std::max<uint8_t>((count + perPage - 1) / perPage, 1);
  if (event == TouchEvents::SwipeUp && page + 1 < pages) {
    page++;
    ShowList();
    return true;
  }
  if (event == TouchEvents::SwipeDown && page > 0) {
    page--;
    ShowList();
    return true;
  }
  return false;
}
