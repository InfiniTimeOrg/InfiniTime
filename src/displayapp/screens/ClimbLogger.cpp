#include "displayapp/screens/ClimbLogger.h"

#include <cstring>
#include <nrf_log.h>
#include "components/motor/MotorController.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      struct CatalogEntry {
        const char* id;
        const char* area;
        const char* colour;
        const char* grade;
        const char* style;
        const char* name;
      };

      // Hard-coded catalog fixture (Roadmap Step 2). Step 3 replaces this
      // with a real read of catalog.csv from LittleFS — the field shape
      // matches CLAUDE.md's catalog record data model so call sites don't
      // need to change. Area names are placeholders (the Castle's real
      // zone list is still an open question); colours/grades are a small
      // mix of boulder (Font-style V-grades) and route (French sport
      // grades) climbs, enough to exercise the area -> colour -> grade
      // filter with more than one option at each step.
      constexpr std::array<CatalogEntry, 8> climbCatalog {{
        {"C001", "Main Hall", "Red", "6a", "Route", "12"},
        {"C002", "Main Hall", "Red", "6b", "Route", "14"},
        {"C003", "Main Hall", "Blue", "V3", "Boulder", "3"},
        {"C004", "Main Hall", "Yellow", "V4", "Boulder", "9"},
        {"C005", "Above & Beyond", "Green", "6c", "Route", "2"},
        {"C006", "Above & Beyond", "Green", "V2", "Boulder", "5"},
        {"C007", "Above & Beyond", "Blue", "6a+", "Route", "7"},
        {"C008", "Above & Beyond", "Red", "V5", "Boulder", "1"},
      }};
    }
  }
}

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonMatrixEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<ClimbLogger*>(obj->user_data);
    screen->OnButtonMatrixEvent(obj, event);
  }

  void LogButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<ClimbLogger*>(obj->user_data);
    screen->OnLogButtonEvent(obj, event);
  }

  // Adds `value` to `out` if it isn't already present (case-sensitive
  // string compare, since catalog fields are plain C strings, not
  // guaranteed to be the same literal/address across entries).
  bool AddIfNew(std::array<const char*, ClimbLogger::kMaxOptions>& out, size_t& count, const char* value) {
    for (size_t i = 0; i < count; i++) {
      if (std::strcmp(out[i], value) == 0) {
        return false;
      }
    }
    if (count < out.size()) {
      out[count++] = value;
      return true;
    }
    return false;
  }
}

ClimbLogger::ClimbLogger(Controllers::MotorController& motorController, Controllers::DateTime& dateTimeController)
  : motorController {motorController}, dateTimeController {dateTimeController} {
  ShowStep();
}

ClimbLogger::~ClimbLogger() {
  lv_obj_clean(lv_scr_act());
}

size_t ClimbLogger::CollectAreas(std::array<const char*, kMaxOptions>& out) const {
  size_t count = 0;
  for (const auto& entry : climbCatalog) {
    AddIfNew(out, count, entry.area);
  }
  return count;
}

size_t ClimbLogger::CollectColours(std::array<const char*, kMaxOptions>& out) const {
  size_t count = 0;
  for (const auto& entry : climbCatalog) {
    if (std::strcmp(entry.area, selectedArea) == 0) {
      AddIfNew(out, count, entry.colour);
    }
  }
  return count;
}

size_t ClimbLogger::CollectGrades(std::array<const char*, kMaxOptions>& out) const {
  size_t count = 0;
  for (const auto& entry : climbCatalog) {
    if (std::strcmp(entry.area, selectedArea) == 0 && std::strcmp(entry.colour, selectedColour) == 0) {
      AddIfNew(out, count, entry.grade);
    }
  }
  return count;
}

size_t ClimbLogger::CollectMatchingClimbs(std::array<const CatalogEntry*, kMaxOptions>& out) const {
  size_t count = 0;
  for (const auto& entry : climbCatalog) {
    if (std::strcmp(entry.area, selectedArea) == 0 && std::strcmp(entry.colour, selectedColour) == 0 &&
        std::strcmp(entry.grade, selectedGrade) == 0 && count < out.size()) {
      out[count++] = &entry;
    }
  }
  return count;
}

void ClimbLogger::ShowStep() {
  lv_obj_clean(lv_scr_act());
  titleLabel = nullptr;
  buttonMatrix = nullptr;
  logButton = nullptr;

  switch (step) {
    case Step::Area: {
      std::array<const char*, kMaxOptions> options {};
      size_t count = CollectAreas(options);
      ShowOptionsStep("Area", options.data(), count);
      break;
    }
    case Step::Colour: {
      std::array<const char*, kMaxOptions> options {};
      size_t count = CollectColours(options);
      ShowOptionsStep("Colour", options.data(), count);
      break;
    }
    case Step::Grade: {
      std::array<const char*, kMaxOptions> options {};
      size_t count = CollectGrades(options);
      ShowOptionsStep("Grade", options.data(), count);
      break;
    }
    case Step::Climb: {
      std::array<const CatalogEntry*, kMaxOptions> matches {};
      size_t count = CollectMatchingClimbs(matches);
      std::array<const char*, kMaxOptions> options {};
      for (size_t i = 0; i < count; i++) {
        options[i] = matches[i]->name;
      }
      ShowOptionsStep("Climb", options.data(), count);
      break;
    }
    case Step::Result:
      ShowResultStep();
      break;
    case Step::Attempts:
      ShowAttemptsStep();
      break;
  }
}

void ClimbLogger::ShowOptionsStep(const char* title, const char* const* options, size_t count) {
  titleLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(titleLabel, title);
  lv_obj_align(titleLabel, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);

  size_t mapIndex = 0;
  for (size_t i = 0; i < count && mapIndex + 1 < optionsMap.size(); i++) {
    optionsMap[mapIndex++] = options[i];
    optionsMap[mapIndex++] = "\n";
  }
  if (mapIndex > 0) {
    mapIndex--; // drop the trailing row separator before the terminator
  }
  optionsMap[mapIndex] = "";

  buttonMatrix = lv_btnmatrix_create(lv_scr_act(), nullptr);
  buttonMatrix->user_data = this;
  lv_obj_set_event_cb(buttonMatrix, ButtonMatrixEventHandler);
  lv_btnmatrix_set_map(buttonMatrix, optionsMap.data());
  lv_obj_set_size(buttonMatrix, 200, 180);
  lv_obj_align(buttonMatrix, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
}

void ClimbLogger::ShowResultStep() {
  titleLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(titleLabel, "Result");
  lv_obj_align(titleLabel, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);

  static constexpr const char* resultMap[] = {"Flash", "Send", "\n", "Fell", "Project", ""};

  buttonMatrix = lv_btnmatrix_create(lv_scr_act(), nullptr);
  buttonMatrix->user_data = this;
  lv_obj_set_event_cb(buttonMatrix, ButtonMatrixEventHandler);
  lv_btnmatrix_set_map(buttonMatrix, const_cast<const char**>(resultMap));
  lv_obj_set_size(buttonMatrix, 200, 180);
  lv_obj_align(buttonMatrix, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
}

void ClimbLogger::ShowAttemptsStep() {
  titleLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(titleLabel, "Attempts");
  lv_obj_align(titleLabel, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);

  // Create() (re)builds the LVGL widgets against the current screen;
  // SetValue() must come after, since it writes into the label Create()
  // just made, and resets the count left over from a previous climb.
  attemptsCounter.Create();
  attemptsCounter.SetValue(1);
  lv_obj_align(attemptsCounter.GetObject(), nullptr, LV_ALIGN_CENTER, 0, -10);

  logButton = lv_btn_create(lv_scr_act(), nullptr);
  logButton->user_data = this;
  lv_obj_set_event_cb(logButton, LogButtonEventHandler);
  lv_obj_set_size(logButton, 120, 50);
  lv_obj_align(logButton, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -10);

  lv_obj_t* logLabel = lv_label_create(logButton, nullptr);
  lv_label_set_text_static(logLabel, "Log");
}

void ClimbLogger::OnButtonMatrixEvent(lv_obj_t* obj, lv_event_t event) {
  // VALUE_CHANGED (not PRESSED) matters here: lv_btnmatrix fires it once,
  // on release, once the press/release gesture has fully resolved against
  // the *current* button matrix. This handler rebuilds the whole screen
  // (destroying this exact button matrix) in response, so reacting to
  // PRESSED instead re-triggers against the freshly-created matrix at the
  // same coordinates while the input device is still mid-gesture, cascading
  // through several steps on a single tap.
  if (obj != buttonMatrix || event != LV_EVENT_VALUE_CHANGED) {
    return;
  }
  const char* text = lv_btnmatrix_get_active_btn_text(buttonMatrix);
  if (text == nullptr) {
    return;
  }
  OnOptionSelected(text);
  ShowStep();
}

void ClimbLogger::OnLogButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (obj != logButton || event != LV_EVENT_CLICKED) {
    return;
  }
  LogAndReset();
}

void ClimbLogger::OnOptionSelected(const char* text) {
  switch (step) {
    case Step::Area:
      selectedArea = text;
      step = Step::Colour;
      break;

    case Step::Colour:
      selectedColour = text;
      step = Step::Grade;
      break;

    case Step::Grade: {
      selectedGrade = text;
      std::array<const CatalogEntry*, kMaxOptions> matches {};
      size_t count = CollectMatchingClimbs(matches);
      if (count <= 1) {
        selectedClimb = count == 1 ? matches[0] : nullptr;
        step = Step::Result;
      } else {
        step = Step::Climb;
      }
      break;
    }

    case Step::Climb: {
      std::array<const CatalogEntry*, kMaxOptions> matches {};
      size_t count = CollectMatchingClimbs(matches);
      for (size_t i = 0; i < count; i++) {
        if (std::strcmp(matches[i]->name, text) == 0) {
          selectedClimb = matches[i];
          break;
        }
      }
      step = Step::Result;
      break;
    }

    case Step::Result:
      if (std::strcmp(text, "Flash") == 0) {
        selectedResult = Result::Flash;
      } else if (std::strcmp(text, "Send") == 0) {
        selectedResult = Result::Send;
      } else if (std::strcmp(text, "Fell") == 0) {
        selectedResult = Result::Fell;
      } else if (std::strcmp(text, "Project") == 0) {
        selectedResult = Result::Project;
      }
      step = Step::Attempts;
      break;

    case Step::Attempts:
      break;
  }
}

const char* ClimbLogger::ToResultString(Result result) {
  switch (result) {
    case Result::Flash:
      return "flash";
    case Result::Send:
      return "send";
    case Result::Fell:
      return "fell";
    case Result::Project:
      return "project";
  }
  return "";
}

void ClimbLogger::LogAndReset() {
  if (selectedClimb != nullptr) {
    // Step 4 replaces this stdout line with an appended row in log.csv,
    // in the same field order as CLAUDE.md's denormalised log record.
    NRF_LOG_INFO("ClimbLogger: %04d-%02d-%02d %02d:%02d:%02d,%s,%s,%s,%s,%s,%s,%d",
                 dateTimeController.Year(),
                 static_cast<int>(dateTimeController.Month()),
                 dateTimeController.Day(),
                 dateTimeController.Hours(),
                 dateTimeController.Minutes(),
                 dateTimeController.Seconds(),
                 selectedClimb->id,
                 selectedClimb->area,
                 selectedClimb->colour,
                 selectedClimb->grade,
                 selectedClimb->style,
                 ToResultString(selectedResult),
                 attemptsCounter.GetValue());
  }

  motorController.RunForDuration(30);

  step = Step::Area;
  selectedArea = nullptr;
  selectedColour = nullptr;
  selectedGrade = nullptr;
  selectedClimb = nullptr;
  selectedResult = Result::Send;

  ShowStep();
}
