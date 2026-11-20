#include "displayapp/screens/WatchFaceBinary.h"

#include <lvgl/lvgl.h>
#include <cstdio>

#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/settings/Settings.h"

using namespace Pinetime::Applications::Screens;

lv_obj_t* WatchFaceBinary::CreateLed(
  lv_obj_t* parent,
  lv_coord_t x,
  lv_coord_t y,
  lv_coord_t size,
  lv_color_t color
) {
  lv_obj_t* led = lv_led_create(parent, nullptr);

  lv_obj_set_size(led, size, size);
  lv_obj_set_pos(led, x, y);

  lv_obj_set_style_local_bg_color(
    led, LV_LED_PART_MAIN, LV_STATE_DEFAULT, color
  );

  lv_obj_set_style_local_bg_opa(
    led, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER
  );

  lv_obj_set_style_local_radius(
    led, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE
  );

  lv_led_set_bright(led, LV_LED_BRIGHT_MAX);
  lv_led_off(led);

  return led;
}

void WatchFaceBinary::CreateTimeLeds() {
  lv_obj_t* screen = lv_scr_act();
  for (size_t col = 0; col < 6; col++) {
    size_t max_rows = MAX_ROWS[col];

    for (size_t row = 0; row < max_rows; row++) {
      leds[col][row] = CreateLed(
        screen,
        COL_START_X - col * COL_SPACING,
        COL_START_Y - row * ROW_SPACING,
        LED_SIZE,
        LED_OFF_COLOR
      );
    }
  }
}

WatchFaceBinary::WatchFaceBinary(
  Controllers::DateTime& dateTimeController,
  Controllers::Settings& settingsController,
  const Controllers::Battery& batteryController,
  const Controllers::Ble& bleController,
  const Controllers::AlarmController& alarmController,
  Controllers::NotificationManager& notificationManager,
  Controllers::HeartRateController& heartRateController,
  Controllers::MotionController& motionController,
  Controllers::SimpleWeatherService& weatherService
) :
  currentDateTime {{}},
  dateTimeController {dateTimeController},
  settingsController {settingsController},
  notificationManager {notificationManager},
  heartRateController {heartRateController},
  motionController {motionController},
  weatherService {weatherService},
  statusIcons(batteryController, bleController, alarmController)
{
  statusIcons.Create();

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  weatherIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_set_style_local_text_font(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
  lv_label_set_text(weatherIcon, "");
  lv_obj_align(weatherIcon, nullptr, LV_ALIGN_IN_TOP_MID, -20, 50);
  lv_obj_set_auto_realign(weatherIcon, true);

  temperature = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_label_set_text(temperature, "");
  lv_obj_align(temperature, nullptr, LV_ALIGN_IN_TOP_MID, 20, 50);

  heartbeatIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(heartbeatIcon, Symbols::heartBeat);
  lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x990000));
  lv_obj_align(heartbeatIcon, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  heartbeatValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(heartbeatValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x990000));
  lv_label_set_text_static(heartbeatValue, "");
  lv_obj_align(heartbeatValue, heartbeatIcon, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

  stepValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x990000));
  lv_label_set_text_static(stepValue, "0");
  lv_obj_align(stepValue, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);

  stepIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x990000));
  lv_label_set_text_static(stepIcon, Symbols::shoe);
  lv_obj_align(stepIcon, stepValue, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  labelTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelTime, lv_scr_act(), LV_ALIGN_CENTER, 0, -84);
  lv_obj_set_style_local_text_color(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));

  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_CENTER, 0, 78);
  lv_obj_set_style_local_text_color(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));

  lv_style_init(&styleLine);
  lv_style_set_line_width(&styleLine, LV_STATE_DEFAULT, 2);
  lv_style_set_line_color(&styleLine, LV_STATE_DEFAULT, lv_color_hex(0x333333));

  hmLine = lv_line_create(lv_scr_act(), NULL);
  lv_line_set_points(hmLine, hmLine_points, 2);
  lv_obj_add_style(hmLine, LV_LINE_PART_MAIN, &styleLine);

  msLine = lv_line_create(lv_scr_act(), NULL);
  lv_line_set_points(msLine, msLine_points, 2);
  lv_obj_add_style(msLine, LV_LINE_PART_MAIN, &styleLine);

  CreateTimeLeds();

  pmLed = CreateLed(
    lv_scr_act(),
    COL_START_X - 5 * COL_SPACING,
    COL_START_Y - 3 * ROW_SPACING,
    LED_SIZE,
    PM_LED_OFF_COLOR
  );

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceBinary::~WatchFaceBinary() {
  if (taskRefresh != nullptr) {
    lv_task_del(taskRefresh);
  }

  lv_obj_clean(lv_scr_act());
}

void WatchFaceBinary::Refresh() {
  statusIcons.Update();

  currentDateTime = std::chrono::time_point_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime());

  if (currentDateTime.IsUpdated()) {
    uint8_t hour   = dateTimeController.Hours();
    uint8_t minute = dateTimeController.Minutes();
    uint8_t second = dateTimeController.Seconds();

    bool pm = hour >= 12;
    bool h12Clock = settingsController.GetClockType() == Controllers::Settings::ClockType::H12;

    if (h12Clock) {
      if (hour == 0) {
        hour = 12;
      } else if (hour > 12) {
        hour = hour - 12;
      }
    }

    uint8_t hourTens   = hour / 10;
    uint8_t hourOnes   = hour % 10;
    uint8_t minuteTens = minute / 10;
    uint8_t minuteOnes = minute % 10;
    uint8_t secondTens = second / 10;
    uint8_t secondOnes = second % 10;

    uint8_t timeDigits[6] = {
      secondOnes,
      secondTens,
      minuteOnes,
      minuteTens,
      hourOnes,
      hourTens
    };

    h12Clock
      ? lv_label_set_text_fmt(labelTime, "%02d:%02d:%02d %s", hour, minute, second, pm ? "PM" : "AM")
      : lv_label_set_text_fmt(labelTime, "%02d:%02d:%02d", hour, minute, second);

    lv_obj_realign(labelTime);

    lv_color_t pmLedColor;
    if (pm) {
      lv_led_on(pmLed);
      pmLedColor = PM_LED_ON_COLOR;
    } else {
      lv_led_off(pmLed);
      pmLedColor = PM_LED_OFF_COLOR;
    }

    lv_obj_set_style_local_bg_color(
      pmLed,
      LV_LED_PART_MAIN,
      LV_STATE_DEFAULT,
      pmLedColor
    );

    lv_color_t ledColor;
    for (size_t col = 0; col < 6; col++) {
      size_t maxRows = MAX_ROWS[col];
      for (size_t row = 0; row < maxRows; row++) {
        bool ledOn = ((1 << row) & timeDigits[col]) > 0;

        if (ledOn) {
          ledColor = LED_ON_COLOR;
          lv_led_on(leds[col][row]);
        } else {
          ledColor = LED_OFF_COLOR;
          lv_led_off(leds[col][row]);
        }

        lv_obj_set_style_local_bg_color(
          leds[col][row],
          LV_LED_PART_MAIN,
          LV_STATE_DEFAULT,
          ledColor
        );
      }
    }

    currentDate = std::chrono::time_point_cast<std::chrono::days>(currentDateTime.Get());
    if (currentDate.IsUpdated()) {
      uint16_t year = dateTimeController.Year();
      uint8_t day   = dateTimeController.Day();
      if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
        lv_label_set_text_fmt(
          labelDate,
          "%s %d %s %d",
          dateTimeController.DayOfWeekShortToString(),
          day,
          dateTimeController.MonthShortToString(),
          year
        );
      } else {
        lv_label_set_text_fmt(
          labelDate,
          "%s %s %d %d",
          dateTimeController.DayOfWeekShortToString(),
          dateTimeController.MonthShortToString(),
          day,
          year
        );
      }
      lv_obj_realign(labelDate);
    }
  }


  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
      lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
    } else {
      lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
      lv_label_set_text_static(heartbeatValue, "");
    }

    lv_obj_realign(heartbeatIcon);
    lv_obj_realign(heartbeatValue);
  }

  stepCount = motionController.NbSteps();
  if (stepCount.IsUpdated()) {
    lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
    lv_obj_realign(stepValue);
    lv_obj_realign(stepIcon);
  }

  currentWeather = weatherService.Current();
  if (currentWeather.IsUpdated()) {
    auto optCurrentWeather = currentWeather.Get();
    if (optCurrentWeather) {
      int16_t temp = optCurrentWeather->temperature.Celsius();
      char tempUnit = 'C';
      if (settingsController.GetWeatherFormat() == Controllers::Settings::WeatherFormat::Imperial) {
        temp = optCurrentWeather->temperature.Fahrenheit();
        tempUnit = 'F';
      }
      lv_label_set_text_fmt(temperature, "%d°%c", temp, tempUnit);
      lv_label_set_text(weatherIcon, Symbols::GetSymbol(optCurrentWeather->iconId, weatherService.IsNight()));
    } else {
      lv_label_set_text_static(temperature, "");
      lv_label_set_text(weatherIcon, "");
    }
    lv_obj_realign(temperature);
    lv_obj_realign(weatherIcon);
  }
}
