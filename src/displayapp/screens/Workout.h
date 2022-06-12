#pragma once

#include "displayapp/screens/Screen.h"
#include "systemtask/SystemTask.h"
#include "displayapp/LittleVgl.h"

#include "components/fs/FS.h"
#include "components/motor/MotorController.h"
#include "components/timer/TimerController.h"
#include "components/datetime/DateTimeController.h"
#include "components/heartrate/HeartRateController.h"

namespace Pinetime::Applications::Screens {

  class Workout : public Screen {
  public:
    Workout(DisplayApp* app,
            Controllers::FS& fsController,
            Controllers::DateTime& dateTimeController,
            Controllers::TimerController& timerController,
            Controllers::MotorController& motorController,
            Controllers::HeartRateController& hrController,
            Pinetime::System::SystemTask& systemTask);

    ~Workout() override;

    void Refresh() override;

    void OnButtonEvent(lv_obj_t* obj, lv_event_t event);

    void OnTimerDone();

  private:
    enum class Mode { Workout, Timer1, Timer2, Timer1Editing, Timer2Editing };

    struct Settings {
      uint8_t timer1Seconds = 0;
      uint8_t timer1Minutes = 1;

      uint8_t timer2Seconds = 0;
      uint8_t timer2Minutes = 2;

      uint8_t hrMax = 0;
      uint8_t hrMin = 0;

      bool timer1Running = false;
      bool timer2Running = false;
      bool workoutInProgress = false;

      TickType_t workoutStartTick = 0;

      Mode currentMode = Mode::Workout;
    };

    Controllers::FS& fsController;
    Controllers::DateTime& dateTimeController;
    Controllers::TimerController& timerController;
    Controllers::MotorController& motorController;
    Controllers::HeartRateController& hrController;

    Pinetime::System::SystemTask& systemTask;

    Settings settings;

    const uint8_t vibrationDurationMs = 1500;

    const char* strTimeUp = "+";
    const char* strTimeDown = "-";
    const char* strTimeFormat = "%02d:%02d";

    const char* strHrFormatMax = "Max:  %d";
    const char* strHrFormatMin = "Min:  %d";
    const char* strHrFormatCurr = "Curr: %d";

    const char* settingsFileName = "/workout_settings.dat";

    uint8_t restSeconds = 0;
    uint8_t restMinutes = 0;

    lv_obj_t* txtRestTime = nullptr;
    lv_obj_t* txtGlobalTime = nullptr;
    lv_obj_t* txtWorkoutTime = nullptr;

    lv_obj_t* txtHrMax = nullptr;
    lv_obj_t* txtHrMin = nullptr;
    lv_obj_t* txtHrCurr = nullptr;

    lv_obj_t *btnRestTimeMinUp = nullptr, *txtRestTimeMinUp = nullptr;
    lv_obj_t *btnRestTimeMinDown = nullptr, *txtRestTimeMinDown = nullptr;

    lv_obj_t *btnRestTimeSecUp = nullptr, *txtRestTimeSecUp = nullptr;
    lv_obj_t *btnRestTimeSecDown = nullptr, *txtRestTimeSecDown = nullptr;

    lv_obj_t *btnMain = nullptr, *txtMain = nullptr;

    lv_obj_t *btnModeTimer1 = nullptr, *txtModeTimer1 = nullptr;
    lv_obj_t *btnModeTimer2 = nullptr, *txtModeTimer2 = nullptr;
    lv_obj_t *btnModeWorkout = nullptr, *txtModeWorkout = nullptr;

    lv_style_t buttonStyleMode;

    lv_task_t* taskRefresh;

    void ReadSettings();

    void WriteSettings();

    void InitButton(lv_obj_t** button, lv_obj_t** label, const char* text, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs);

    void InitButtonStyleMode(lv_style_t style);

    void AddRestTimeButtons();

    void RemoveRestTimeButtons();

    void AddModeSelectionButtons();

    void RemoveModeSelectionButtons();

    void AddHrText();

    void RemoveHrText();

    void SetTimeToLabel(lv_obj_t* label, uint8_t higherUnit, uint8_t smallerUnit);

    void SetMode(Mode mode);

    void OnRestTimeUpdated();

    void OnWorkoutTimeUpdated();

    void OnScreenStateUpdated();

    void OnWorkoutStateUpdated();
  };
}