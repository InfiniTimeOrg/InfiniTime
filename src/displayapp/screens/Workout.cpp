#include "displayapp/screens/Workout.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

static void btnEventHandler(lv_obj_t* obj, lv_event_t event) {
  auto screen = static_cast<Workout*>(obj->user_data);
  screen->OnButtonEvent(obj, event);
}

Workout::Workout(DisplayApp* app,
                 Controllers::FS& fsController,
                 Controllers::DateTime& dateTimeController,
                 Controllers::TimerController& timerController,
                 Controllers::MotorController& motorController,
                 Controllers::HeartRateController& hrController,
                 Pinetime::System::SystemTask& systemTask)
  : Screen(app),
    fsController {fsController}, 
    dateTimeController {dateTimeController},
    timerController {timerController}, 
    motorController {motorController},
    hrController {hrController},
    systemTask {systemTask} {

  txtGlobalTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(txtGlobalTime, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  txtRestTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(txtRestTime, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 140);

  InitButtonStyleMode(buttonStyleMode);
  InitButton(&btnMain, &txtMain, "", LV_ALIGN_IN_BOTTOM_LEFT, 0, -20);
  
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  ReadSettings();
  OnWorkoutStateUpdated();

  if (!timerController.IsRunning()) {
    if (settings.timer1Running) {
      settings.timer1Running = false;
    }
    if (settings.timer2Running) {
      settings.timer2Running = false;
    }
  }

  SetMode(settings.currentMode);
}

Workout::~Workout() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Workout::Refresh() {
  SetTimeToLabel(txtGlobalTime, dateTimeController.Hours(), dateTimeController.Minutes());

  if (settings.workoutInProgress) {
    OnWorkoutTimeUpdated();
  }
  if (settings.currentMode == Mode::Timer1Editing || settings.currentMode == Mode::Timer2Editing) {
    return;
  }

  if (timerController.IsRunning()) {
    if (settings.currentMode == Mode::Timer1Editing || settings.currentMode == Mode::Timer2Editing) {
      return;
    }
    if (settings.currentMode == Mode::Timer1 && !settings.timer1Running) {
      return;
    }
    if (settings.currentMode == Mode::Timer2 && !settings.timer2Running) {
      return;
    }

    uint32_t seconds = timerController.GetTimeRemaining() / 1000;

    restSeconds = seconds % 60;
    restMinutes = seconds / 60;

    OnRestTimeUpdated();
  }

  uint8_t currHr = hrController.HeartRate();
  if (settings.hrMin > currHr || settings.hrMin == 0) {
    settings.hrMin = currHr;
  }
  if (settings.hrMax < currHr) {
    settings.hrMax = currHr;
  }

  if (txtHrMax != nullptr) {
    lv_label_set_text_fmt(txtHrMax, strHrFormatMax, settings.hrMax);
  }
  if (txtHrMin != nullptr) {
    lv_label_set_text_fmt(txtHrMin, strHrFormatMin, settings.hrMin);
  }
  if (txtHrCurr != nullptr) {
    lv_label_set_text_fmt(txtHrCurr, strHrFormatCurr, currHr);
  }
}

void Workout::OnRestTimeUpdated() {
  SetTimeToLabel(txtRestTime, restMinutes, restSeconds);
}

void Workout::OnScreenStateUpdated() {
  const char* mainButtonText;

  switch (settings.currentMode) {
    case Mode::Timer1:
      if (settings.timer1Running) {
        mainButtonText = "Pause";
      } else {
        mainButtonText = "Start";
      }
      break;
    
    case Mode::Timer2:
      if (settings.timer2Running) {
        mainButtonText = "Pause";
      } else {
        mainButtonText = "Start";
      }
      break;

    case Mode::Workout:
      if (settings.workoutInProgress) {
        mainButtonText = "Stop";
      } else {
        mainButtonText = "Start";
      }
      break;

    case Mode::Timer1Editing:
    case Mode::Timer2Editing:
      mainButtonText = "Set";
      break;
  
    default:
      mainButtonText = "";
      break;
  }

  lv_label_set_text(txtMain, mainButtonText);
}

void Workout::OnWorkoutStateUpdated() {
  if (settings.workoutInProgress) {
    hrController.Start();
    systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);

    if (txtWorkoutTime == nullptr) {
      txtWorkoutTime = lv_label_create(lv_scr_act(), nullptr);
      lv_obj_align(txtWorkoutTime, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -15, 0);
    } 
  } else {
    hrController.Stop();
    systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);

    if (txtWorkoutTime != nullptr) {
      lv_obj_del(txtWorkoutTime);
      txtWorkoutTime = nullptr;
    }
  }
}

void Workout::InitButton(lv_obj_t** button, 
                         lv_obj_t** label, 
                         const char* text, 
                         lv_align_t align, 
                         lv_coord_t x_ofs, 
                         lv_coord_t y_ofs) {
  *button = lv_btn_create(lv_scr_act(), nullptr);
  (*button)->user_data = this;

  lv_obj_align(*button, lv_scr_act(), align, x_ofs, y_ofs);
  lv_obj_add_style(*button, LV_OBJ_PART_MAIN, &buttonStyleMode);

  lv_obj_set_width(*button, 70);
  lv_obj_set_height(*button, 50);

  lv_obj_set_event_cb(*button, btnEventHandler);

  *label = lv_label_create(*button, nullptr);
  lv_label_set_text(*label, text);
}

void Workout::InitButtonStyleMode(lv_style_t style) {
  lv_style_init(&style);
  lv_style_set_bg_color(&style, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lv_style_set_bg_color(&style, LV_STATE_CHECKED, LV_COLOR_YELLOW);
}

void Workout::AddRestTimeButtons() {
  InitButton(&btnRestTimeMinUp, &txtRestTimeMinUp, strTimeUp, LV_ALIGN_IN_LEFT_MID, 20, -80);
  InitButton(&btnRestTimeMinDown, &txtRestTimeMinDown, strTimeDown, LV_ALIGN_IN_LEFT_MID, 20, +40);

  InitButton(&btnRestTimeSecUp, &txtRestTimeSecUp, strTimeUp, LV_ALIGN_IN_RIGHT_MID, 10, -80);
  InitButton(&btnRestTimeSecDown, &txtRestTimeSecDown, strTimeDown, LV_ALIGN_IN_RIGHT_MID, 10, +40);
}

void Workout::RemoveRestTimeButtons() {
  if (btnRestTimeSecUp != nullptr) {
    lv_obj_del(btnRestTimeSecUp);
    btnRestTimeSecUp = nullptr;
  }
  if (btnRestTimeSecDown != nullptr) {
    lv_obj_del(btnRestTimeSecDown);
    btnRestTimeSecDown = nullptr;
  }
  if (btnRestTimeMinUp != nullptr) {
    lv_obj_del(btnRestTimeMinUp);
    btnRestTimeMinUp = nullptr;
  }
  if (btnRestTimeMinDown != nullptr) {
    lv_obj_del(btnRestTimeMinDown);
    btnRestTimeMinDown = nullptr;
  }
}

void Workout::AddModeSelectionButtons() {
  if (btnModeTimer1 != nullptr || btnModeTimer2 != nullptr || btnModeWorkout != nullptr) {
    return;
  }

  InitButton(&btnModeTimer1, &txtModeTimer1, "T1", LV_ALIGN_IN_RIGHT_MID, 0, 0);
  InitButton(&btnModeTimer2, &txtModeTimer2, "T2", LV_ALIGN_IN_RIGHT_MID, 0, 60);
  InitButton(&btnModeWorkout, &txtModeWorkout, "W", LV_ALIGN_IN_RIGHT_MID, 0, -60);
}

void Workout::RemoveModeSelectionButtons() {
  if (btnModeTimer1 != nullptr) {
    lv_obj_del(btnModeTimer1);
    btnModeTimer1 = nullptr;
  }
  if (btnModeTimer2 != nullptr) {
    lv_obj_del(btnModeTimer2);
    btnModeTimer2 = nullptr;
  }
  if (btnModeWorkout != nullptr) {
    lv_obj_del(btnModeWorkout);
    btnModeWorkout = nullptr;
  }
}

void Workout::SetTimeToLabel(lv_obj_t* label, uint8_t biggerUnit, uint8_t smallerUnit) {
  lv_label_set_text_fmt(label, strTimeFormat, biggerUnit, smallerUnit);
}

void Workout::OnButtonEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnMain) {
      switch (settings.currentMode) {
        case Mode::Timer1:
          if (timerController.IsRunning()) {
            timerController.StopTimer();
          }

          settings.timer1Running = !settings.timer1Running;
          settings.timer2Running = false;

          if (settings.timer1Running) {
            timerController.StartTimer((restSeconds + restMinutes * 60) * 1000);
          }
          break;

        case Mode::Timer2:
          if (timerController.IsRunning()) {
            timerController.StopTimer();
          }

          settings.timer1Running = false;
          settings.timer2Running = !settings.timer2Running;

          if (settings.timer2Running) {
            timerController.StartTimer((restSeconds + restMinutes * 60) * 1000);
          }
          break;

        case Mode::Timer1Editing:
          settings.timer1Seconds = restSeconds;
          settings.timer1Minutes = restMinutes;

          SetMode(Mode::Timer1);
          break;

        case Mode::Timer2Editing:
          settings.timer2Seconds = restSeconds;
          settings.timer2Minutes = restMinutes;

          SetMode(Mode::Timer2);
          break;

        case Mode::Workout:
          if (timerController.IsRunning()) {
            timerController.StopTimer();
          }

          settings.workoutInProgress = !settings.workoutInProgress;

          OnWorkoutStateUpdated();

          if (settings.workoutInProgress) {
            settings.workoutStartTick = xTaskGetTickCount();

            settings.hrMax = 0;
            settings.hrMin = 0;
          }

          settings.timer1Running = false;
          settings.timer2Running = false;

          restSeconds = 0;
          restMinutes = 0;

          break;
      
        default:
          break;
      }

      WriteSettings();
      OnScreenStateUpdated();
    } else if (obj == btnRestTimeMinUp) {
      if (restMinutes >= 59) {
        restMinutes = 0;
      } else {
        ++restMinutes;
      }

      OnRestTimeUpdated();
    } else if (obj == btnRestTimeMinDown) {
      if (restMinutes == 0) {
        restMinutes = 59;
      } else {
        --restMinutes;
      }

      OnRestTimeUpdated();
    } else if (obj == btnRestTimeSecUp) {
      if (restSeconds >= 59) {
        restSeconds = 0;
      } else {
        ++restSeconds;
      }

      OnRestTimeUpdated();
    } else if (obj == btnRestTimeSecDown) {
      if (restSeconds == 0) {
        restSeconds = 59;
      } else {
        --restSeconds;
      }

      OnRestTimeUpdated();
    } else if (obj == btnModeTimer1) {
      SetMode(Mode::Timer1);
    } else if (obj == btnModeTimer2) {
      SetMode(Mode::Timer2);
    } else if (obj == btnModeWorkout) {
      SetMode(Mode::Workout);
    }
  } else if (event == LV_EVENT_LONG_PRESSED) {
    if (obj == btnModeTimer1) {
      if (settings.timer1Running && timerController.IsRunning()) {
        timerController.StopTimer();
        settings.timer1Running = false;
      }

      SetMode(Mode::Timer1Editing);
    } else if (obj == btnModeTimer2) {
      if (settings.timer2Running && timerController.IsRunning()) {
        timerController.StopTimer();
        settings.timer2Running = false;
      }

      SetMode(Mode::Timer2Editing);
    }
  }
}

void Workout::SetMode(Mode mode) {
  settings.currentMode = mode;

  if (mode == Mode::Timer1Editing || mode == Mode::Timer2Editing) {
    RemoveHrText();
    RemoveModeSelectionButtons();
    AddRestTimeButtons();

    lv_obj_align(txtRestTime, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 5, -35);
    lv_obj_set_style_local_text_font(txtRestTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);

    lv_obj_align(btnMain, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  } else {
    AddHrText();
    RemoveRestTimeButtons();
    AddModeSelectionButtons();

    lv_obj_align(txtRestTime, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 140);
    lv_obj_set_style_local_text_font(txtRestTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);

    lv_obj_align(btnMain, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, -20);
  }

  lv_obj_set_state(btnModeTimer1, mode == Mode::Timer1 ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
  lv_obj_set_state(btnModeTimer2, mode == Mode::Timer2 ? LV_STATE_CHECKED : LV_STATE_DEFAULT);
  lv_obj_set_state(btnModeWorkout, mode == Mode::Workout ? LV_STATE_CHECKED : LV_STATE_DEFAULT);

  if (mode == Mode::Timer1Editing || (mode == Mode::Timer1 && !settings.timer1Running)) {
    restSeconds = settings.timer1Seconds;
    restMinutes = settings.timer1Minutes;
  } else if (mode == Mode::Timer2Editing || (mode == Mode::Timer2 && !settings.timer2Running)) {
    restSeconds = settings.timer2Seconds;
    restMinutes = settings.timer2Minutes;
  }

  WriteSettings();
  OnRestTimeUpdated();
  OnScreenStateUpdated();
}

void Workout::AddHrText() {
  if (txtHrMax == nullptr) {
    txtHrMax = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_align(txtHrMax, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 60);
  }
  if (txtHrMin == nullptr) {
    txtHrMin = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_align(txtHrMin, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 80);
  }
  if (txtHrCurr == nullptr) {
    txtHrCurr = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_align(txtHrCurr, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 100);
  }
}

void Workout::RemoveHrText() {
  if (txtHrMax != nullptr) {
    lv_obj_del(txtHrMax);
    txtHrMax = nullptr;
  }
  if (txtHrMin != nullptr) {
    lv_obj_del(txtHrMin);
    txtHrMin = nullptr;
  }
  if (txtHrCurr != nullptr) {
    lv_obj_del(txtHrCurr);
    txtHrCurr = nullptr;
  }
}

void Workout::OnWorkoutTimeUpdated() {
  if (txtWorkoutTime == nullptr) return;

  TickType_t currentTime  = xTaskGetTickCount();
  TickType_t delta = 0;

  // Take care of overflow
  if (settings.workoutStartTick > currentTime) {
    delta = 0xffffffff - settings.workoutStartTick;
    delta += (currentTime + 1);
  } else {
    delta = currentTime - settings.workoutStartTick;
  }
    
  const int timeElapsedMillis = static_cast<float>(delta) / static_cast<float>(configTICK_RATE_HZ);

  const int secs = timeElapsedMillis % 60;
  const int mins = timeElapsedMillis / 60;

  SetTimeToLabel(txtWorkoutTime, mins, secs);
}

void Workout::OnTimerDone() {
  settings.timer1Running = false;
  settings.timer2Running = false;

  if (settings.currentMode == Mode::Timer1) {
    restSeconds = settings.timer1Seconds;
    restMinutes = settings.timer1Minutes;
  } else if (settings.currentMode == Mode::Timer2) {
    restSeconds = settings.timer2Seconds;
    restMinutes = settings.timer2Minutes;
  }

  motorController.RunForDuration(vibrationDurationMs);

  if (settings.currentMode != Mode::Timer1Editing && settings.currentMode != Mode::Timer2Editing) {
    OnRestTimeUpdated();
    OnScreenStateUpdated();
  }
}

void Workout::ReadSettings() {
  Settings buffer;
  lfs_file_t settingsFile;

  if (fsController.FileOpen(&settingsFile, settingsFileName, LFS_O_RDONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    return;
  }
  fsController.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&buffer), sizeof(settings));
  fsController.FileClose(&settingsFile);

  this->settings = buffer;
}

void Workout::WriteSettings() {
  lfs_file_t settingsFile;

  if (fsController.FileOpen(&settingsFile, settingsFileName, LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    return;
  }
  fsController.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
  fsController.FileClose(&settingsFile);
}