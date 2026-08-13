#include "displayapp/screens/ClimbLogger.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iterator>
#include <nrf_log.h>
#include "components/motor/MotorController.h"
#include "components/datetime/DateTimeController.h"
#include "components/fs/FS.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // Placeholder option lists (Roadmap Step 2). Gyms/grade ladders are
  // still open questions (see ROADMAP.md's cross-cutting open questions
  // table) — these are enough to exercise the flow, not the final word.
  constexpr const char* gymOptions[] = {"The Castle", "Climbing District", "Other"};
  constexpr const char* styleOptions[] = {"Boulder", "Top Rope", "Lead"};
  constexpr const char* typeOptions[] = {"Slab", "Overhang", "Vertical", "Mixed"};
  constexpr const char* vScaleGrades[] = {"V0", "V1", "V2", "V3", "V4", "V5", "V6", "V7"};
  constexpr const char* fontScaleGrades[] = {"5+", "6a", "6a+", "6b", "6b+", "6c", "6c+", "7a"};
  constexpr const char* attemptOptions[] = {"Send", "Did Not Finish"};

  void ButtonMatrixEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<ClimbLogger*>(obj->user_data);
    screen->OnButtonMatrixEvent(obj, event);
  }
}

ClimbLogger::ClimbLogger(Controllers::MotorController& motorController,
                          Controllers::DateTime& dateTimeController,
                          Controllers::FS& filesystem)
  : motorController {motorController}, dateTimeController {dateTimeController}, filesystem {filesystem} {
  ShowStep();
}

ClimbLogger::~ClimbLogger() {
  lv_obj_clean(lv_scr_act());
}

bool ClimbLogger::IsBoulderStyle() const {
  return selectedStyle != nullptr && std::strcmp(selectedStyle, "Boulder") == 0;
}

void ClimbLogger::ShowStep() {
  lv_obj_clean(lv_scr_act());
  titleLabel = nullptr;
  buttonMatrix = nullptr;

  switch (step) {
    case Step::Gym:
      ShowOptionsStep("Gym", gymOptions, std::size(gymOptions), selectedGym);
      break;
    case Step::Style:
      ShowOptionsStep("Style", styleOptions, std::size(styleOptions), selectedStyle);
      break;
    case Step::Type:
      ShowOptionsStep("Type", typeOptions, std::size(typeOptions), selectedType);
      break;
    case Step::Grade:
      if (IsBoulderStyle()) {
        ShowOptionsStep("Grade (V)", vScaleGrades, std::size(vScaleGrades), selectedGrade);
      } else {
        ShowOptionsStep("Grade (Font)", fontScaleGrades, std::size(fontScaleGrades), selectedGrade);
      }
      break;
    case Step::Attempt:
      ShowOptionsStep("Attempt", attemptOptions, std::size(attemptOptions), selectedAttempt);
      break;
  }
}

void ClimbLogger::ShowOptionsStep(const char* title, const char* const* options, size_t count, const char* rememberedValue) {
  titleLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(titleLabel, title);
  lv_obj_align(titleLabel, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);

  // One button per row reads clearly for a few options; longer lists (the
  // grade ladders) go two-per-row so buttons stay a reasonable tap size.
  const size_t perRow = count > 4 ? 2 : 1;

  size_t mapIndex = 0;
  int rememberedBtnIndex = -1;
  for (size_t i = 0; i < count && mapIndex + 1 < optionsMap.size(); i++) {
    if (rememberedValue != nullptr && std::strcmp(options[i], rememberedValue) == 0) {
      rememberedBtnIndex = static_cast<int>(i);
    }
    optionsMap[mapIndex++] = options[i];
    if ((i + 1) % perRow == 0 || i + 1 == count) {
      optionsMap[mapIndex++] = "\n";
    }
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

  // Row separators don't consume a button index, so options[i] lines up
  // directly with button index i regardless of how rows were wrapped.
  if (rememberedBtnIndex >= 0) {
    lv_btnmatrix_set_one_check(buttonMatrix, true);
    lv_btnmatrix_set_btn_ctrl(buttonMatrix, static_cast<uint16_t>(rememberedBtnIndex), LV_BTNMATRIX_CTRL_CHECK_STATE);
  }
}

void ClimbLogger::OnButtonMatrixEvent(lv_obj_t* obj, lv_event_t event) {
  // CLICKED matters here, not PRESSED or (perhaps counter-intuitively)
  // VALUE_CHANGED. lv_btnmatrix's ctrl_bits default LV_BTNMATRIX_CTRL_CLICK_TRIG
  // to off (see allocate_btn_areas_and_controls's memset), and with that bit
  // off, lv_btnmatrix fires its own VALUE_CHANGED signal on *press*, same
  // timing problem as PRESSED. CLICKED, in contrast, is sent generically by
  // lv_indev.c itself only once a press/release gesture has fully resolved
  // against the *current* object. This handler rebuilds the whole screen
  // (destroying this exact button matrix) in response to a selection, so
  // anything that can fire on press-down re-triggers against the
  // freshly-created matrix at the same coordinates while the input device
  // is still mid-gesture, cascading through several steps on one tap.
  if (obj != buttonMatrix || event != LV_EVENT_CLICKED) {
    return;
  }
  const char* text = lv_btnmatrix_get_active_btn_text(buttonMatrix);
  if (text == nullptr) {
    return;
  }
  OnOptionSelected(text);
}

void ClimbLogger::OnOptionSelected(const char* text) {
  switch (step) {
    case Step::Gym:
      selectedGym = text;
      step = Step::Style;
      break;

    case Step::Style:
      selectedStyle = text;
      step = Step::Type;
      break;

    case Step::Type:
      selectedType = text;
      step = Step::Grade;
      break;

    case Step::Grade:
      selectedGrade = text;
      step = Step::Attempt;
      break;

    case Step::Attempt:
      selectedAttempt = text;
      LogAndReset();
      return; // LogAndReset() already redraws the next screen
  }
  ShowStep();
}

void ClimbLogger::LogAndReset() {
  WriteLogEntry();
  motorController.RunForDuration(30);

  // Deliberately not clearing selectedGym/selectedStyle/selectedType/
  // selectedGrade/selectedAttempt: they double as next log's remembered
  // defaults, pre-checked in ShowOptionsStep.
  step = Step::Gym;
  ShowStep();
}

void ClimbLogger::WriteLogEntry() {
  // UTC, ISO 8601 ("Z" = zero offset) -- deliberately not local time.
  // dateTimeController.Year()/Hours()/etc. read `localTime`, adjusted by
  // whatever timezone/DST offset the companion app last sent over BLE
  // (0 if it never has, e.g. in the simulator); UTCDateTime() undoes that
  // adjustment. Storing UTC keeps the log unambiguous regardless of
  // where/whether that offset was ever set correctly -- local-time
  // display, if wanted, is a host-side concern for whatever ingests
  // log.csv later (Step 6), not something to bake into the file.
  const std::time_t utcTimeT = std::chrono::system_clock::to_time_t(dateTimeController.UTCDateTime());
  const std::tm* utcTm = std::gmtime(&utcTimeT);

  // Same field order as CLAUDE.md's denormalised log record (timestamp,
  // then a copy of each catalog-ish field so the row still means
  // something after Gym/Style/Type/Grade option lists change later).
  char line[128];
  int len = std::snprintf(line,
                           sizeof(line),
                           "%04d-%02d-%02dT%02d:%02d:%02dZ,%s,%s,%s,%s,%s",
                           utcTm->tm_year + 1900,
                           utcTm->tm_mon + 1,
                           utcTm->tm_mday,
                           utcTm->tm_hour,
                           utcTm->tm_min,
                           utcTm->tm_sec,
                           selectedGym,
                           selectedStyle,
                           selectedType,
                           selectedGrade,
                           selectedAttempt);
  if (len <= 0) {
    return;
  }
  const size_t writeLen = static_cast<size_t>(len) < sizeof(line) ? static_cast<size_t>(len) : sizeof(line) - 1;

  // Directory may not exist yet on a fresh filesystem — same
  // open-then-create-on-failure idiom AlarmController uses for
  // /.system before writing into it.
  lfs_dir_t climbsDir;
  if (filesystem.DirOpen("/climbs", &climbsDir) != LFS_ERR_OK) {
    filesystem.DirCreate("/climbs");
  }
  filesystem.DirClose(&climbsDir);

  lfs_file_t logFile;
  if (filesystem.FileOpen(&logFile, "/climbs/log.csv", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[ClimbLogger] Failed to open log.csv for appending");
    return;
  }
  filesystem.FileWrite(&logFile, reinterpret_cast<const uint8_t*>(line), writeLen);
  filesystem.FileWrite(&logFile, reinterpret_cast<const uint8_t*>("\n"), 1);
  filesystem.FileClose(&logFile);

  // Host-visible confirmation that the write actually happened, on top of
  // (not instead of) the real file write above -- helpful during sim
  // development without needing a make pull-log round-trip every time.
  NRF_LOG_INFO("ClimbLogger: logged %s", line);
}
