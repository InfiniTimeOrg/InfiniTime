/*  Copyright (C) 2025 Asger Gitz-Johansen

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

#include "Apps.h"
#include <components/settings/Settings.h>
#include <displayapp/Controllers.h>
#include <displayapp/LittleVgl.h>
#include <displayapp/screens/Screen.h>
#include <displayapp/screens/Symbols.h>
#include <displayapp/widgets/Counter.h>
#include <systemtask/WakeLock.h>

namespace Pinetime::Applications::Screens {
  class Sleep : public Screen {
  public:
    enum class State : uint8_t { NotTracking, Tracking, Alerting };

    Sleep(Controllers::SleepTrackingController& sleeptrackingController,
          Controllers::Settings::ClockType clockType,
          Controllers::MotorController& motorController,
          System::SystemTask& systemTask,
          DisplayApp& displayApp);
    ~Sleep() override;

    void StartAlerting();

    void SetState(const State& uistate);

    void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
    void OnAlarmValueChanged();

    bool OnTouchEvent(TouchEvents event) override;
    bool OnButtonPushed() override;
    void StopAlerting();

  private:
    void UpdateWakeAlarmTime();

    // Dependencies
    Controllers::SleepTrackingController& sleeptrackingController;
    Controllers::MotorController& motorController;
    System::SystemTask& systemTask;
    DisplayApp& displayApp;
    System::WakeLock wakeLock;

    // UI
    State state;
    Widgets::Counter hourCounter;
    Widgets::Counter minuteCounter;
    lv_obj_t* lblampm;
    lv_obj_t *startButton, *startText;
    lv_obj_t *stopButton, *stopText;
    lv_obj_t *dismissButton, *dismissText;
    lv_obj_t *colonLabel, *wakeUpLabel, *sleepingLabel;
    lv_task_t* taskStopAlarm;
  };
}

namespace Pinetime::Applications {
  template <>
  struct AppTraits<Apps::Sleep> {
    static constexpr Apps app = Apps::Sleep;
    static constexpr const char* icon = Screens::Symbols::bed;

    static Screens::Screen* Create(AppControllers& controllers) {
      return new Screens::Sleep(controllers.sleeptrackingController,
                                controllers.settingsController.GetClockType(),
                                controllers.motorController,
                                *controllers.systemTask,
                                *controllers.displayApp);
    }

    static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
      return true;
    };
  };
}
