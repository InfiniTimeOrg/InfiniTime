#include "displayapp/screens/Sleep.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;
using Night = Pinetime::Controllers::SleepTracker::Night;
using Stage = Pinetime::Controllers::Sleep::Stage;

namespace {
  constexpr uint32_t sameNightSeconds = 3 * 60 * 60;
  constexpr const char* weekdays[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

  struct LocalTime {
    unsigned month;
    unsigned day;
    unsigned weekday;
    int hour;
    int minute;
  };

  LocalTime ToLocal(uint32_t seconds) {
    const std::chrono::sys_seconds time {std::chrono::seconds {seconds}};
    const auto days = std::chrono::floor<std::chrono::days>(time);
    const std::chrono::year_month_day date {days};
    const std::chrono::hh_mm_ss clock {time - days};
    return {static_cast<unsigned>(date.month()),
            static_cast<unsigned>(date.day()),
            std::chrono::weekday {days}.c_encoding(),
            static_cast<int>(clock.hours().count()),
            static_cast<int>(clock.minutes().count())};
  }

  lv_obj_t* CreateLabel(lv_color_t color, lv_coord_t x, lv_coord_t y) {
    lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, color);
    lv_obj_set_pos(label, x, y);
    return label;
  }
}

Sleep::Sleep(Controllers::SleepTracker& tracker) : tracker {tracker} {
  haveLive = tracker.Live(live);
  if (haveLive && tracker.HistoryCount() > 0) {
    const uint32_t stored = tracker.History(0).start;
    const uint32_t distance = stored > live.start ? stored - live.start : live.start - stored;
    // The stored copy of this night is older than what the tracker sees right now
    if (distance < sameNightSeconds) {
      historyOffset = 1;
    }
  }

  lv_obj_t* title = CreateLabel(Colors::neonMagenta, 4, 2);
  lv_label_set_text_static(title, "// SLEEP");

  labelPage = CreateLabel(Colors::neonPurple, 0, 2);
  labelDate = CreateLabel(Colors::neonYellow, 4, 28);
  labelLive = CreateLabel(Colors::neonMagenta, 0, 28);
  lv_label_set_text_static(labelLive, "LIVE");
  lv_obj_align(labelLive, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 28);

  labelAsleep = CreateLabel(Colors::neonCyan, 2, 52);
  lv_obj_set_style_local_text_font(labelAsleep, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  labelAsleepCaption = CreateLabel(Colors::lightGray, 4, 92);
  lv_label_set_text_static(labelAsleepCaption, "ASLEEP");

  labelTimes = CreateLabel(Colors::neonYellow, 0, 56);
  lv_label_set_align(labelTimes, LV_LABEL_ALIGN_RIGHT);

  // Hypnogram: awake at the top, light in the middle, still at the bottom
  graphAxis = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_recolor(graphAxis, true);
  lv_obj_set_style_local_text_line_space(graphAxis, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, -6);
  lv_label_set_text_static(graphAxis, "#ff2a6d W#\n#05d9e8 L#\n#9d4edd S#");
  lv_obj_set_pos(graphAxis, 2, graphTop - 10);

  graph = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_line_color(graph, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, Colors::neonCyan);
  lv_obj_set_style_local_line_width(graph, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_pos(graph, graphX, graphTop);

  labelStats = CreateLabel(LV_COLOR_WHITE, 4, 168);
  lv_label_set_recolor(labelStats, true);

  labelEmpty = CreateLabel(Colors::lightGray, 4, 40);
  lv_label_set_long_mode(labelEmpty, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(labelEmpty, 232);
  lv_label_set_text_static(labelEmpty,
                           "No sleep detected\n"
                           "yet.\n\n"
                           "Wear the watch to\n"
                           "bed, the night\n"
                           "shows up here.");

  ShowPage();
}

Sleep::~Sleep() {
  lv_obj_clean(lv_scr_act());
}

uint8_t Sleep::PageCount() const {
  const uint8_t pages = (haveLive ? 1 : 0) + tracker.HistoryCount() - historyOffset;
  return std::max<uint8_t>(pages, 1);
}

bool Sleep::NightForPage(uint8_t index, Night& night) const {
  if (haveLive) {
    if (index == 0) {
      night = live;
      return true;
    }
    index--;
  }
  const size_t historyIndex = index + historyOffset;
  if (historyIndex >= tracker.HistoryCount()) {
    return false;
  }
  night = tracker.History(historyIndex);
  return true;
}

void Sleep::ShowPage() {
  Night night;
  const bool found = NightForPage(page, night);

  lv_label_set_text_fmt(labelPage, "%d/%d", page + 1, PageCount());
  lv_obj_align(labelPage, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 2);

  lv_obj_set_hidden(labelEmpty, found);
  for (lv_obj_t* obj : {labelDate, labelAsleep, labelAsleepCaption, labelTimes, graph, graphAxis, labelStats}) {
    lv_obj_set_hidden(obj, !found);
  }
  lv_obj_set_hidden(labelLive, !found || night.ongoing == 0);
  if (!found) {
    return;
  }

  const uint32_t end = night.start + night.inBed * 60;
  const LocalTime wake = ToLocal(end);
  const LocalTime fellAsleep = ToLocal(night.start);

  lv_label_set_text_fmt(labelDate, "%s %02u-%02u", weekdays[wake.weekday % 7], wake.month, wake.day);
  lv_label_set_text_fmt(labelAsleep, "%d:%02d", night.asleep / 60, night.asleep % 60);
  if (night.ongoing != 0) {
    lv_label_set_text_fmt(labelTimes, "%02d:%02d\n--:--", fellAsleep.hour, fellAsleep.minute);
  } else {
    lv_label_set_text_fmt(labelTimes, "%02d:%02d\n%02d:%02d", fellAsleep.hour, fellAsleep.minute, wake.hour, wake.minute);
  }
  lv_obj_align(labelTimes, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 56);

  DrawGraph(night);

  const uint16_t light = night.asleep - night.still;
  const unsigned efficiency = night.inBed == 0 ? 0 : (night.asleep * 100U) / night.inBed;
  char heartRate[8];
  char lowest[8];
  if (night.avgHeartRate != 0) {
    snprintf(heartRate, sizeof(heartRate), "%d", night.avgHeartRate);
    snprintf(lowest, sizeof(lowest), "%d", night.minHeartRate);
  } else {
    snprintf(heartRate, sizeof(heartRate), "--");
    snprintf(lowest, sizeof(lowest), "--");
  }
  lv_label_set_text_fmt(labelStats,
                        "#9d4edd STILL# %d:%02d  #ff2a6d HR#  %s\n"
                        "#05d9e8 LIGHT# %d:%02d  #fcee0a LOW# %s\n"
                        "#ff2a6d AWAKE# %d:%02d  #fcee0a EFF# %u%%",
                        night.still / 60,
                        night.still % 60,
                        heartRate,
                        light / 60,
                        light % 60,
                        lowest,
                        night.awake / 60,
                        night.awake % 60,
                        efficiency);
}

void Sleep::DrawGraph(const Night& night) {
  const uint8_t epochs = std::max<uint8_t>(night.epochs, 1);
  uint16_t points = 0;
  for (uint8_t epoch = 0; epoch < night.epochs; epoch++) {
    lv_coord_t y;
    switch (night.EpochStage(epoch)) {
      case Stage::Awake:
        y = 0;
        break;
      case Stage::Still:
        y = 2 * rowHeight;
        break;
      default:
        y = rowHeight;
        break;
    }
    const lv_coord_t x1 = static_cast<lv_coord_t>(epoch * graphWidth / epochs);
    const lv_coord_t x2 = static_cast<lv_coord_t>((epoch + 1) * graphWidth / epochs);
    graphPoints[points++] = {x1, y};
    graphPoints[points++] = {x2, y};
  }
  lv_line_set_points(graph, graphPoints.data(), points);
}

bool Sleep::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::SwipeUp && page + 1 < PageCount()) {
    page++;
    ShowPage();
    return true;
  }
  if (event == TouchEvents::SwipeDown && page > 0) {
    page--;
    ShowPage();
    return true;
  }
  return false;
}
