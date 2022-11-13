#include "displayapp/screens/WatchFaceGravel.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

namespace Style {
  static constexpr lv_style_int_t STEP_LINE_THICKNESS = 8;
  static constexpr lv_style_int_t STEP_LINE_PADDING = 4;

  static constexpr lv_style_int_t BATTERY_INDICATOR_THICKNESS = 8;
  static constexpr lv_style_int_t BATTERY_INDICATOR_LENGTH = 60;
  static constexpr uint32_t BATTERY_CHARGING_ANIMATION_DURATION = 5000;

  static constexpr lv_color_t red = LV_COLOR_MAKE(0xCE, 0x27, 0x2A);
  static constexpr lv_color_t green = LV_COLOR_MAKE(0x20, 0xA8, 0x20);
  static constexpr lv_color_t yellow = LV_COLOR_MAKE(0xCE, 0xCE, 0x27);
  static constexpr lv_color_t blue = LV_COLOR_MAKE(0x27, 0xA2, 0xCE);
  static constexpr lv_color_t barBackground = LV_COLOR_MAKE(0x38, 0x38, 0x38);
}

WatchFaceGravel::WatchFaceGravel(DisplayApp* app,
                                 Controllers::DateTime& dateTimeController,
                                 Controllers::Battery& batteryController,
                                 Controllers::Settings& settingsController,
                                 Controllers::MotionController& motionController,
                                 Controllers::FS& filesystem)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    settingsController {settingsController},
    motionController(motionController) {

  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/SquareRegular_72.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_SquareRegular72 = lv_font_load("F:/fonts/SquareRegular_72.bin");
  }

  if (filesystem.FileOpen(&f, "/fonts/SquareRegular_20.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_SquareRegular20 = lv_font_load("F:/fonts/SquareRegular_20.bin");
  }

  labelTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_SquareRegular72);

  labelTimeAMPM = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(labelTimeAMPM, "");
  lv_obj_set_style_local_text_font(labelTimeAMPM, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_SquareRegular20);
  lv_obj_align(labelTimeAMPM, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -50, -45);

  stepLineBackground = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(stepLineBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_style_local_border_width(stepLineBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Style::STEP_LINE_THICKNESS);
  lv_obj_set_style_local_border_color(stepLineBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Style::barBackground);
  lv_obj_set_style_local_border_opa(stepLineBackground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_width(stepLineBackground, LV_HOR_RES - (Style::STEP_LINE_THICKNESS / 2) - Style::STEP_LINE_PADDING * 2);
  lv_obj_set_height(stepLineBackground, LV_VER_RES - (Style::STEP_LINE_THICKNESS / 2) - Style::STEP_LINE_PADDING * 2);
  lv_obj_align(stepLineBackground, NULL, LV_ALIGN_IN_TOP_LEFT, Style::STEP_LINE_PADDING, Style::STEP_LINE_PADDING);

  stepLine = lv_line_create(lv_scr_act(), nullptr);
  lv_style_init(&styleStepLine);
  lv_style_set_line_width(&styleStepLine, LV_STATE_DEFAULT, Style::STEP_LINE_THICKNESS);
  lv_style_set_line_color(&styleStepLine, LV_STATE_DEFAULT, Style::blue);
  lv_style_set_line_rounded(&styleStepLine, LV_STATE_DEFAULT, true);
  lv_obj_add_style(stepLine, LV_LINE_PART_MAIN, &styleStepLine);
  lv_obj_align(stepLine, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  batteryBar = lv_bar_create(lv_scr_act(), nullptr);
  lv_obj_set_size(batteryBar, Style::BATTERY_INDICATOR_LENGTH, Style::BATTERY_INDICATOR_THICKNESS);
  lv_bar_set_range(batteryBar, 0, 100);
  lv_bar_set_value(batteryBar, 0, LV_ANIM_OFF);
  lv_obj_set_style_local_radius(batteryBar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE);
  lv_obj_set_style_local_bg_color(batteryBar, LV_BAR_PART_BG, LV_STATE_DEFAULT, Style::barBackground);
  lv_obj_set_style_local_bg_opa(batteryBar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_OPA_50);
  lv_obj_set_style_local_bg_color(batteryBar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_align(batteryBar, lv_scr_act(), LV_ALIGN_CENTER, 40, 40);

  lv_anim_init(&animationBatteryCharging);
  lv_anim_set_var(&animationBatteryCharging, batteryBar);
  lv_anim_set_repeat_count(&animationBatteryCharging, LV_ANIM_REPEAT_INFINITE);

  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelDate, batteryBar, LV_ALIGN_OUT_LEFT_MID, -15, 0);
  lv_obj_set_style_local_text_font(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_SquareRegular20);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceGravel::~WatchFaceGravel() {
  lv_task_del(taskRefresh);
  lv_style_reset(&styleStepLine);

  if (font_SquareRegular20 != nullptr) {
    lv_font_free(font_SquareRegular20);
  }

  if (font_SquareRegular72 != nullptr) {
    lv_font_free(font_SquareRegular72);
  }

  lv_obj_clean(lv_scr_act());
}

void WatchFaceGravel::Refresh() {
  currentDateTime = dateTimeController.CurrentDateTime();

  if (currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime - dp);

    uint8_t hour = time.hours().count();
    uint8_t minute = time.minutes().count();

    if (displayedHour != hour || displayedMinute != minute) {
      displayedHour = hour;
      displayedMinute = minute;

      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
        char ampmChar[3] = "AM";
        if (hour == 0) {
          hour = 12;
        } else if (hour == 12) {
          ampmChar[0] = 'P';
        } else if (hour > 12) {
          hour = hour - 12;
          ampmChar[0] = 'P';
        }
        lv_label_set_text(labelTimeAMPM, ampmChar);
      }

      lv_label_set_text_fmt(labelTime, "%02d:%02d", hour, minute);
      lv_obj_align_origo(labelTime, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
    }

    auto yearMonthDay = date::year_month_day(dp);
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>(static_cast<unsigned>(yearMonthDay.month()));
    auto day = static_cast<unsigned>(yearMonthDay.day());
    if ((day != currentDay) || (month != currentMonth)) {
      lv_label_set_text_fmt(labelDate, "%d %s", day, Pinetime::Controllers::DateTime::MonthShortToStringLow(month));
    }
    lv_obj_realign(labelDate);
  }

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    uint16_t pointCount = refreshStepLinePoints(std::min(stepCount.Get(), settingsController.GetStepsGoal()));
    lv_line_set_points(stepLine, stepLinePoints.data(), pointCount);
  }
  lv_obj_realign(stepLine);

  isCharging = batteryController.IsCharging();
  if (isCharging.IsUpdated()) {
    toggleBatteryChargingAnimation(isCharging.Get(), batteryPercentRemaining.Get());
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    if (isCharging.Get()) {
      toggleBatteryChargingAnimation(true, batteryPercentRemaining.Get());
    } else {
      updateBatteryBar(batteryPercentRemaining.Get());
    }
  }
}

void WatchFaceGravel::updateBatteryBar(uint8_t percent) {
  lv_bar_set_value(batteryBar, percent, LV_ANIM_ON);
  if (percent >= 70) {
    lv_obj_set_style_local_bg_color(batteryBar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Style::green);
  } else if (percent >= 40) {
    lv_obj_set_style_local_bg_color(batteryBar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Style::yellow);
  } else {
    lv_obj_set_style_local_bg_color(batteryBar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Style::red);
  }
}

void WatchFaceGravel::toggleBatteryChargingAnimation(bool enabled, uint8_t percent) {
  if (enabled) {
    lv_obj_set_style_local_bg_color(batteryBar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, Style::green);
    lv_anim_del(batteryBar, (lv_anim_exec_xcb_t) lv_bar_set_value);
    lv_anim_set_exec_cb(&animationBatteryCharging, (lv_anim_exec_xcb_t) lv_bar_set_value);
    lv_anim_set_values(&animationBatteryCharging, percent, 100);
    uint32_t duration = (Style::BATTERY_CHARGING_ANIMATION_DURATION / 100) * (100 - percent);
    lv_anim_set_time(&animationBatteryCharging, duration);
    lv_anim_start(&animationBatteryCharging);
  } else {
    lv_anim_del(batteryBar, (lv_anim_exec_xcb_t) lv_bar_set_value);
    updateBatteryBar(batteryPercentRemaining.Get());
  }
}

WatchFaceGravel::DrawStop WatchFaceGravel::getDrawEnd(float percent, uint16_t width, uint16_t height) {
  DrawStop drawStop = {.place = Place::TOP, .location = 0.0f};
  float halfBarWidth = width / 2;

  if (percent > halfBarWidth) {
    float second = percent - (halfBarWidth);
    // right
    if (second > (height - Style::STEP_LINE_THICKNESS)) {
      float third = second - (height - Style::STEP_LINE_THICKNESS);

      // bottom
      if (third > (width - Style::STEP_LINE_THICKNESS)) {
        float forth = third - (width - Style::STEP_LINE_THICKNESS);

        // left
        if (forth > (height - Style::STEP_LINE_THICKNESS)) {
          float fifth = forth - (height - Style::STEP_LINE_THICKNESS);

          // top left
          if (fifth == halfBarWidth) {
            drawStop.place = Place::TOP;
            drawStop.location = halfBarWidth;
          } else {
            drawStop.place = Place::TOP;
            drawStop.location = Style::STEP_LINE_THICKNESS + fifth;
          }
        } else {
          drawStop.place = Place::LEFT;
          drawStop.location = height - Style::STEP_LINE_THICKNESS - forth;
        }

      } else {
        drawStop.place = Place::BOTTOM;
        drawStop.location = width - Style::STEP_LINE_THICKNESS - third;
      }
    } else {
      drawStop.place = Place::RIGHT;
      drawStop.location = Style::STEP_LINE_THICKNESS + second;
    }

  } else {
    drawStop.place = Place::TOP;
    drawStop.location = halfBarWidth + percent;
  }

  return drawStop;
}

// Credit to https://github.com/mrwonderman/android-square-progressbar for original source
uint16_t WatchFaceGravel::refreshStepLinePoints(uint32_t stepCount) {
  uint16_t currentPointIndex = 0;

  if (stepCount == 0)
    return currentPointIndex;

  int barWidth = LV_HOR_RES - Style::STEP_LINE_PADDING;
  int barHeight = LV_VER_RES - Style::STEP_LINE_PADDING;

  float scope = (2 * barWidth) + (2 * barHeight) - (4 * Style::STEP_LINE_THICKNESS);
  float halfStrokeWidth = Style::STEP_LINE_THICKNESS / 2 + Style::STEP_LINE_PADDING;

  const auto addPoint = [&](lv_coord_t x, lv_coord_t y) {
    stepLinePoints[currentPointIndex++] = {.x = x, .y = y};
  };

  float progress = static_cast<float>(stepCount * 100) / static_cast<float>(settingsController.GetStepsGoal());
  float percent = (scope / 100.0F) * progress;

  auto drawEnd = getDrawEnd(percent, barWidth, barHeight);

  if (drawEnd.place == Place::TOP) {
    if (drawEnd.location > (barWidth / 2) && progress < 100.0F) {
      addPoint(barWidth / 2, halfStrokeWidth);
      addPoint(drawEnd.location, halfStrokeWidth);
    } else {
      addPoint(barWidth / 2, halfStrokeWidth);
      addPoint(barWidth - halfStrokeWidth, halfStrokeWidth);
      addPoint(barWidth - halfStrokeWidth, barHeight - halfStrokeWidth);
      addPoint(halfStrokeWidth, barHeight - halfStrokeWidth);
      addPoint(halfStrokeWidth, halfStrokeWidth);
      addPoint(Style::STEP_LINE_THICKNESS, halfStrokeWidth);
      addPoint(drawEnd.location, halfStrokeWidth);
    }
    return currentPointIndex;
  }

  if (drawEnd.place == Place::RIGHT) {
    addPoint(barWidth / 2, halfStrokeWidth);
    addPoint(barWidth - halfStrokeWidth, halfStrokeWidth);
    addPoint(barWidth - halfStrokeWidth, 0 + drawEnd.location);
    return currentPointIndex;
  }

  if (drawEnd.place == Place::BOTTOM) {
    addPoint(barWidth / 2, halfStrokeWidth);
    addPoint(barWidth - halfStrokeWidth, halfStrokeWidth);
    addPoint(barWidth - halfStrokeWidth, barHeight - halfStrokeWidth);
    addPoint(barWidth - Style::STEP_LINE_THICKNESS, barHeight - halfStrokeWidth);
    addPoint(drawEnd.location, barHeight - halfStrokeWidth);
    return currentPointIndex;
  }

  if (drawEnd.place == Place::LEFT) {
    addPoint(barWidth / 2, halfStrokeWidth);
    addPoint(barWidth - halfStrokeWidth, halfStrokeWidth);
    addPoint(barWidth - halfStrokeWidth, barHeight - halfStrokeWidth);
    addPoint(halfStrokeWidth, barHeight - halfStrokeWidth);
    addPoint(halfStrokeWidth, barHeight - Style::STEP_LINE_THICKNESS);
    addPoint(halfStrokeWidth, drawEnd.location);
    return currentPointIndex;
  }

  return currentPointIndex;
}

bool WatchFaceGravel::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};

  if (filesystem.FileOpen(&file, "/fonts/SquareRegular_72.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);

  if (filesystem.FileOpen(&file, "/fonts/SquareRegular_20.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);
  return true;
}
