#include "displayapp/screens/Weather.h"

#include <lvgl/lvgl.h>
#include <ctime>

#include "components/ble/SimpleWeatherService.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/WeatherSymbols.h"
#include "displayapp/screens/Label.h"
#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/InfiniTimeTheme.h"
#include "utility/DirtyValue.h"

using namespace Pinetime::Applications::Screens;

namespace {
  uint8_t TemperatureStyle(Pinetime::Controllers::SimpleWeatherService::Temperature temp) {
    if (temp.Celsius() <= 0) { // freezing
      return LV_TABLE_PART_CELL3;
    } else if (temp.Celsius() <= 4) { // ice
      return LV_TABLE_PART_CELL4;
    } else if (temp.Celsius() >= 27) { // hot
      return LV_TABLE_PART_CELL6;
    }
    return LV_TABLE_PART_CELL5; // normal
  }

  void FormatTime(uint16_t minutesSinceMidnight, char* buf, size_t size, Pinetime::Controllers::Settings::ClockType clockType) {
    uint8_t hours = minutesSinceMidnight / 60;
    uint8_t mins = minutesSinceMidnight % 60;
    if (clockType == Pinetime::Controllers::Settings::ClockType::H12) {
      const char* ampm = (hours < 12) ? "AM" : "PM";
      hours = hours % 12;
      if (hours == 0) {
        hours = 12;
      }
      snprintf(buf, size, "%d:%02d %s", hours, mins, ampm);
    } else {
      snprintf(buf, size, "%02d:%02d", hours, mins);
    }
  }

  void FormatLastUpdated(uint64_t weatherTs, uint64_t nowTs, char* buf, size_t size) {
    if (nowTs < weatherTs) {
      snprintf(buf, size, "unknown");
      return;
    }
    auto deltaSec = nowTs - weatherTs;
    auto deltaMin = deltaSec / 60;
    auto deltaHrs = deltaMin / 60;
    if (deltaMin < 1) {
      snprintf(buf, size, "just now");
    } else if (deltaMin < 60) {
      snprintf(buf, size, "%d min ago", static_cast<int>(deltaMin));
    } else if (deltaHrs < 24) {
      snprintf(buf, size, "%d hr%s ago", static_cast<int>(deltaHrs), deltaHrs > 1 ? "s" : "");
    } else {
      snprintf(buf, size, ">24 hrs ago");
    }
  }
}

// Weather main screen: manages 3 pages
Weather::Weather(DisplayApp* app,
                 Pinetime::Controllers::Settings& settingsController,
                 Pinetime::Controllers::SimpleWeatherService& weatherService,
                 Pinetime::Controllers::DateTime& dateTimeController)
  : app {app},
    settingsController {settingsController},
    weatherService {weatherService},
    dateTimeController {dateTimeController},
    screens {app,
             0,
             {[this]() -> std::unique_ptr<Screen> {
                return CreateScreen1();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen2();
              },
              [this]() -> std::unique_ptr<Screen> {
                return CreateScreen3();
              }},
             Screens::ScreenListModes::UpDown} {
}

Weather::~Weather() {
  lv_obj_clean(lv_scr_act());
}

bool Weather::OnTouchEvent(TouchEvents event) {
  return screens.OnTouchEvent(event);
}

// Page 1: Current Weather with location and last updated
namespace {
  class WeatherPage1 : public Screen {
  public:
    WeatherPage1(Pinetime::Controllers::Settings& settings,
                 Pinetime::Controllers::SimpleWeatherService& weather,
                 Pinetime::Controllers::DateTime& dateTime)
      : settingsController {settings}, weatherService {weather}, dateTimeController {dateTime}, pageIndicator(0, 3) {

      icon = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_text_font(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
      lv_label_set_text(icon, "");
      lv_obj_align(icon, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);
      lv_obj_set_auto_realign(icon, true);

      condition = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(condition, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
      lv_label_set_text(condition, "");
      lv_obj_align(condition, icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
      lv_obj_set_auto_realign(condition, true);

      temperature = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_text_font(temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
      lv_label_set_text(temperature, "---");
      lv_obj_align(temperature, nullptr, LV_ALIGN_CENTER, 0, -10);
      lv_obj_set_auto_realign(temperature, true);

      minMaxTemp = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(minMaxTemp, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
      lv_label_set_text(minMaxTemp, "");
      lv_obj_align(minMaxTemp, temperature, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
      lv_obj_set_auto_realign(minMaxTemp, true);

      location = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(location, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_width(location, LV_HOR_RES - 20);
      lv_label_set_text(location, "");
      lv_obj_align(location, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -35);
      lv_obj_set_auto_realign(location, true);

      lastUpdated = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(lastUpdated, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
      lv_label_set_text(lastUpdated, "");
      lv_obj_align(lastUpdated, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
      lv_obj_set_auto_realign(lastUpdated, true);

      pageIndicator.Create();

      taskRefresh = lv_task_create(RefreshTaskCallback, 1000, LV_TASK_PRIO_MID, this);
      Refresh();
    }

    ~WeatherPage1() override {
      lv_task_del(taskRefresh);
      lv_obj_clean(lv_scr_act());
    }

    void Refresh() override {
      currentWeather = weatherService.Current();
      if (currentWeather.IsUpdated()) {
        auto optCurrentWeather = currentWeather.Get();
        if (optCurrentWeather) {
          int16_t temp = optCurrentWeather->temperature.Celsius();
          int16_t minTemp = optCurrentWeather->minTemperature.Celsius();
          int16_t maxTemp = optCurrentWeather->maxTemperature.Celsius();
          char tempUnit = 'C';
          if (settingsController.GetWeatherFormat() == Pinetime::Controllers::Settings::WeatherFormat::Imperial) {
            temp = optCurrentWeather->temperature.Fahrenheit();
            minTemp = optCurrentWeather->minTemperature.Fahrenheit();
            maxTemp = optCurrentWeather->maxTemperature.Fahrenheit();
            tempUnit = 'F';
          }
          lv_obj_set_style_local_text_color(temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, optCurrentWeather->temperature.Color());
          lv_label_set_text(icon, Symbols::GetSymbol(optCurrentWeather->iconId, weatherService.IsNight()));
          lv_label_set_text(condition, Symbols::GetCondition(optCurrentWeather->iconId));
          lv_label_set_text_fmt(temperature, "%d°%c", temp, tempUnit);
          lv_label_set_text_fmt(minMaxTemp, "%d° / %d°", minTemp, maxTemp);
          lv_label_set_text(location, optCurrentWeather->location.data());

          // Show last updated time
          uint64_t currentTime = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime().time_since_epoch()).count());
          char updateBuf[20];
          FormatLastUpdated(optCurrentWeather->timestamp, currentTime, updateBuf, sizeof(updateBuf));
          lv_label_set_text_fmt(lastUpdated, "Updated: %s", updateBuf);
        } else {
          lv_label_set_text(icon, "");
          lv_label_set_text(condition, "");
          lv_label_set_text(temperature, "---");
          lv_obj_set_style_local_text_color(temperature, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
          lv_label_set_text(minMaxTemp, "");
          lv_label_set_text(location, "");
          lv_label_set_text(lastUpdated, "");
        }
      }
    }

  private:
    Pinetime::Controllers::Settings& settingsController;
    Pinetime::Controllers::SimpleWeatherService& weatherService;
    Pinetime::Controllers::DateTime& dateTimeController;

    Pinetime::Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

    lv_obj_t* icon;
    lv_obj_t* condition;
    lv_obj_t* temperature;
    lv_obj_t* minMaxTemp;
    lv_obj_t* location;
    lv_obj_t* lastUpdated;
    lv_task_t* taskRefresh;

    Pinetime::Applications::Widgets::PageIndicator pageIndicator;
  };
}

std::unique_ptr<Screen> Weather::CreateScreen1() {
  return std::make_unique<WeatherPage1>(settingsController, weatherService, dateTimeController);
}

// Page 2: 5-Day Forecast
namespace {
  class WeatherPage2 : public Screen {
  public:
    WeatherPage2(Pinetime::Controllers::Settings& settings, Pinetime::Controllers::SimpleWeatherService& weather)
      : settingsController {settings}, weatherService {weather}, pageIndicator(1, 3) {

      // Header
      lv_obj_t* header = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(header, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
      lv_label_set_text_static(header, "Forecast");
      lv_obj_align(header, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);

      forecast = lv_table_create(lv_scr_act(), nullptr);
      lv_table_set_col_cnt(forecast, Pinetime::Controllers::SimpleWeatherService::MaxNbForecastDays);
      lv_table_set_row_cnt(forecast, 4);
      // LV_TABLE_PART_CELL1: Default table style
      lv_obj_set_style_local_border_color(forecast, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, LV_COLOR_BLACK);
      lv_obj_set_style_local_text_color(forecast, LV_TABLE_PART_CELL1, LV_STATE_DEFAULT, Colors::lightGray);
      // LV_TABLE_PART_CELL2: Condition icon
      lv_obj_set_style_local_border_color(forecast, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, LV_COLOR_BLACK);
      lv_obj_set_style_local_text_color(forecast, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_text_font(forecast, LV_TABLE_PART_CELL2, LV_STATE_DEFAULT, &fontawesome_weathericons);
      // LV_TABLE_PART_CELL3: Freezing
      lv_obj_set_style_local_border_color(forecast, LV_TABLE_PART_CELL3, LV_STATE_DEFAULT, LV_COLOR_BLACK);
      lv_obj_set_style_local_text_color(forecast, LV_TABLE_PART_CELL3, LV_STATE_DEFAULT, Colors::blue);
      // LV_TABLE_PART_CELL4: Ice
      lv_obj_set_style_local_border_color(forecast, LV_TABLE_PART_CELL4, LV_STATE_DEFAULT, LV_COLOR_BLACK);
      lv_obj_set_style_local_text_color(forecast, LV_TABLE_PART_CELL4, LV_STATE_DEFAULT, LV_COLOR_CYAN);
      // LV_TABLE_PART_CELL5: Normal
      lv_obj_set_style_local_border_color(forecast, LV_TABLE_PART_CELL5, LV_STATE_DEFAULT, LV_COLOR_BLACK);
      lv_obj_set_style_local_text_color(forecast, LV_TABLE_PART_CELL5, LV_STATE_DEFAULT, Colors::orange);
      // LV_TABLE_PART_CELL6: Hot
      lv_obj_set_style_local_border_color(forecast, LV_TABLE_PART_CELL6, LV_STATE_DEFAULT, LV_COLOR_BLACK);
      lv_obj_set_style_local_text_color(forecast, LV_TABLE_PART_CELL6, LV_STATE_DEFAULT, Colors::deepOrange);

      lv_obj_align(forecast, nullptr, LV_ALIGN_IN_LEFT_MID, 0, 10);

      for (int i = 0; i < Pinetime::Controllers::SimpleWeatherService::MaxNbForecastDays; i++) {
        lv_table_set_col_width(forecast, i, 48);
        lv_table_set_cell_type(forecast, 1, i, LV_TABLE_PART_CELL2);
        lv_table_set_cell_align(forecast, 0, i, LV_LABEL_ALIGN_CENTER);
        lv_table_set_cell_align(forecast, 1, i, LV_LABEL_ALIGN_CENTER);
        lv_table_set_cell_align(forecast, 2, i, LV_LABEL_ALIGN_CENTER);
        lv_table_set_cell_align(forecast, 3, i, LV_LABEL_ALIGN_CENTER);
      }

      pageIndicator.Create();

      taskRefresh = lv_task_create(RefreshTaskCallback, 1000, LV_TASK_PRIO_MID, this);
      Refresh();
    }

    ~WeatherPage2() override {
      lv_task_del(taskRefresh);
      lv_obj_clean(lv_scr_act());
    }

    void Refresh() override {
      currentForecast = weatherService.GetForecast();
      if (currentForecast.IsUpdated()) {
        auto optCurrentForecast = currentForecast.Get();
        if (optCurrentForecast) {
          std::tm localTime = *std::localtime(reinterpret_cast<const time_t*>(&optCurrentForecast->timestamp));

          for (int i = 0; i < optCurrentForecast->nbDays; i++) {
            int16_t maxTemp = optCurrentForecast->days[i]->maxTemperature.Celsius();
            int16_t minTemp = optCurrentForecast->days[i]->minTemperature.Celsius();
            if (settingsController.GetWeatherFormat() == Pinetime::Controllers::Settings::WeatherFormat::Imperial) {
              maxTemp = optCurrentForecast->days[i]->maxTemperature.Fahrenheit();
              minTemp = optCurrentForecast->days[i]->minTemperature.Fahrenheit();
            }
            lv_table_set_cell_type(forecast, 2, i, TemperatureStyle(optCurrentForecast->days[i]->maxTemperature));
            lv_table_set_cell_type(forecast, 3, i, TemperatureStyle(optCurrentForecast->days[i]->minTemperature));
            uint8_t wday = localTime.tm_wday + i + 1;
            if (wday > 7) {
              wday -= 7;
            }
            const char* dayOfWeek =
              Pinetime::Controllers::DateTime::DayOfWeekShortToStringLow(static_cast<Pinetime::Controllers::DateTime::Days>(wday));
            lv_table_set_cell_value(forecast, 0, i, dayOfWeek);
            lv_table_set_cell_value(forecast, 1, i, Symbols::GetSymbol(optCurrentForecast->days[i]->iconId, false));
            // Pad cells based on the largest number of digits on each column
            char maxPadding[3] = "  ";
            char minPadding[3] = "  ";
            int diff = snprintf(nullptr, 0, "%d", maxTemp) - snprintf(nullptr, 0, "%d", minTemp);
            if (diff <= 0) {
              maxPadding[-diff] = '\0';
              minPadding[0] = '\0';
            } else {
              maxPadding[0] = '\0';
              minPadding[diff] = '\0';
            }
            lv_table_set_cell_value_fmt(forecast, 2, i, "%s%d", maxPadding, maxTemp);
            lv_table_set_cell_value_fmt(forecast, 3, i, "%s%d", minPadding, minTemp);
          }
        } else {
          for (int i = 0; i < Pinetime::Controllers::SimpleWeatherService::MaxNbForecastDays; i++) {
            lv_table_set_cell_value(forecast, 0, i, "");
            lv_table_set_cell_value(forecast, 1, i, "");
            lv_table_set_cell_value(forecast, 2, i, "");
            lv_table_set_cell_value(forecast, 3, i, "");
            lv_table_set_cell_type(forecast, 2, i, LV_TABLE_PART_CELL1);
            lv_table_set_cell_type(forecast, 3, i, LV_TABLE_PART_CELL1);
          }
        }
      }
    }

  private:
    Pinetime::Controllers::Settings& settingsController;
    Pinetime::Controllers::SimpleWeatherService& weatherService;

    Pinetime::Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::Forecast>> currentForecast {};

    lv_obj_t* forecast;
    lv_task_t* taskRefresh;

    Pinetime::Applications::Widgets::PageIndicator pageIndicator;
  };
}

std::unique_ptr<Screen> Weather::CreateScreen2() {
  return std::make_unique<WeatherPage2>(settingsController, weatherService);
}

// Page 3: Sunrise/Sunset with arc
namespace {
  // Animation callback for arc value
  void SetArcValue(void* obj, lv_anim_value_t value) {
    lv_arc_set_value(static_cast<lv_obj_t*>(obj), value);
  }

  class WeatherPage3 : public Screen {
  public:
    WeatherPage3(Pinetime::Controllers::Settings& settings,
                 Pinetime::Controllers::SimpleWeatherService& weather,
                 Pinetime::Controllers::DateTime& dateTime)
      : settingsController {settings}, weatherService {weather}, dateTimeController {dateTime}, pageIndicator(2, 3) {

      // Header
      lv_obj_t* header = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(header, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
      lv_label_set_text_static(header, "Sun Times");
      lv_obj_align(header, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);

      // Create the arc showing the sun's position relative to the daylight period
      sunArc = lv_arc_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_bg_opa(sunArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, LV_OPA_0);
      lv_obj_set_style_local_line_color(sunArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, Colors::bgAlt);
      lv_obj_set_style_local_border_width(sunArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 2);
      lv_obj_set_style_local_radius(sunArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 0);
      lv_obj_set_style_local_line_color(sunArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, Colors::orange);
      lv_arc_set_bg_angles(sunArc, 180, 0);
      lv_obj_set_size(sunArc, 180, 180);
      lv_obj_align(sunArc, nullptr, LV_ALIGN_CENTER, 0, 10);

      // Initialize animation with ease-in-out path
      lv_anim_init(&anim);
      lv_anim_set_var(&anim, sunArc);
      lv_anim_set_exec_cb(&anim, SetArcValue);
      lv_anim_set_time(&anim, 1000); // 1 second animation
      static const lv_anim_path_t ease_in_out_path = {.cb = lv_anim_path_ease_in_out};
      lv_anim_set_path(&anim, &ease_in_out_path);

      // Sunrise icon and time widgets
      sunriseIcon = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(sunriseIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_text_font(sunriseIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
      lv_label_set_text(sunriseIcon, "");

      sunriseTime = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(sunriseTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
      lv_label_set_text(sunriseTime, "");

      // Sunset icon and time widgets
      sunsetIcon = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(sunsetIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      lv_obj_set_style_local_text_font(sunsetIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &fontawesome_weathericons);
      lv_label_set_text(sunsetIcon, "");

      sunsetTime = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(sunsetTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
      lv_label_set_text(sunsetTime, "");

      // Current time
      currentTimeLabel = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_set_style_local_text_color(currentTimeLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
      lv_obj_set_style_local_text_font(currentTimeLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
      lv_label_set_text(currentTimeLabel, "--:--");
      lv_obj_align(currentTimeLabel, nullptr, LV_ALIGN_CENTER, 0, 10);
      lv_obj_set_auto_realign(currentTimeLabel, true);

      pageIndicator.Create();

      taskRefresh = lv_task_create(RefreshTaskCallback, 1000, LV_TASK_PRIO_MID, this);
      Refresh();
    }

    ~WeatherPage3() override {
      lv_anim_del(sunArc, nullptr);
      lv_task_del(taskRefresh);
      lv_obj_clean(lv_scr_act());
    }

    void Refresh() override {
      auto optWeather = weatherService.Current();
      if (!optWeather || optWeather->sunrise == 0 || optWeather->sunset == 0) {
        lv_arc_set_range(sunArc, 0, 1);
        lv_arc_set_value(sunArc, 0);
        lv_label_set_text(currentTimeLabel, "--:--");
        return;
      }

      // Show current time, but also get the hours/minutes to calculate sun position
      lv_label_set_text(currentTimeLabel, dateTimeController.FormattedTime().c_str());
      uint16_t currentMinutes = (dateTimeController.Hours() * 60) + dateTimeController.Minutes();

      uint16_t sunrise = optWeather->sunrise;
      uint16_t sunset = optWeather->sunset;

      // Format sunrise/sunset times
      char sunriseBuf[12];
      char sunsetBuf[12];
      FormatTime(sunrise, sunriseBuf, sizeof(sunriseBuf), settingsController.GetClockType());
      FormatTime(sunset, sunsetBuf, sizeof(sunsetBuf), settingsController.GetClockType());

      // Arc always shows today's daylight period (sunrise to sunset)
      lv_arc_set_range(sunArc, sunrise, sunset);

      // Calculate target arc value based on current time
      int16_t targetValue;
      if (currentMinutes < sunrise) {
        // Before sunrise: arc at 0%
        targetValue = sunrise;
      } else if (currentMinutes > sunset) {
        // After sunset: arc at 100%
        targetValue = sunset;
      } else {
        // During daylight: arc shows progress
        targetValue = currentMinutes;
      }

      // Animate arc only on first refresh
      if (!arcInitialized) {
        arcInitialized = true;
        lv_anim_set_values(&anim, sunrise, targetValue);
        lv_anim_start(&anim);
      } else {
        lv_arc_set_value(sunArc, targetValue);
      }

      // Sunrise on left
      lv_label_set_text(sunriseIcon, Symbols::GetSymbol(Pinetime::Controllers::SimpleWeatherService::Icons::Sun, false));
      lv_obj_align(sunriseIcon, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 30, -70);
      lv_label_set_text(sunriseTime, sunriseBuf);
      lv_obj_align(sunriseTime, sunriseIcon, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

      // Sunset on right
      lv_label_set_text(sunsetIcon, Symbols::GetSymbol(Pinetime::Controllers::SimpleWeatherService::Icons::Sun, true));
      lv_obj_align(sunsetIcon, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, -30, -70);
      lv_label_set_text(sunsetTime, sunsetBuf);
      lv_obj_align(sunsetTime, sunsetIcon, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    }

  private:
    Pinetime::Controllers::Settings& settingsController;
    Pinetime::Controllers::SimpleWeatherService& weatherService;
    Pinetime::Controllers::DateTime& dateTimeController;

    lv_obj_t* sunArc;
    lv_obj_t* sunriseIcon;
    lv_obj_t* sunriseTime;
    lv_obj_t* sunsetIcon;
    lv_obj_t* sunsetTime;
    lv_obj_t* currentTimeLabel;
    lv_task_t* taskRefresh;
    lv_anim_t anim;
    bool arcInitialized = false;

    Pinetime::Applications::Widgets::PageIndicator pageIndicator;
  };
}

std::unique_ptr<Screen> Weather::CreateScreen3() {
  return std::make_unique<WeatherPage3>(settingsController, weatherService, dateTimeController);
}
