/*  Copyright (C) 2021 mruss77, Florian

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

#include "displayapp/screens/Screen.h"
#include "systemtask/SystemTask.h"
#include "displayapp/LittleVgl.h"
#include "components/alarm/AlarmController.h"
#include "displayapp/widgets/Counter.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Alarm : public Screen {
      public:
        Alarm(Controllers::AlarmController& alarmController,
              Controllers::Settings::ClockType clockType,
              System::SystemTask& systemTask,
              Controllers::MotorController& motorController);
        ~Alarm() override;
        void SetAlerting();
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        void OnValueChanged();
        void StopAlerting();

      private:
        Controllers::AlarmController& alarmController;
        System::SystemTask& systemTask;
        Controllers::MotorController& motorController;

        lv_obj_t *btnStop, *txtStop, *btnRecur, *txtRecur, *btnInfo, *enableSwitch;
        lv_obj_t* lblampm = nullptr;
        lv_obj_t* txtMessage = nullptr;
        lv_obj_t* btnMessage = nullptr;
        lv_task_t* taskStopAlarm = nullptr;

        enum class EnableButtonState { On, Off, Alerting };
        void DisableAlarm();
        void SetRecurButtonState();
        void SetSwitchState(lv_anim_enable_t anim);
        void SetAlarm();
        void ShowInfo();
        void HideInfo();
        void ToggleRecurrence();
        void UpdateAlarmTime();
        Widgets::Counter hourCounter = Widgets::Counter(0, 23, jetbrains_mono_76);
        Widgets::Counter minuteCounter = Widgets::Counter(0, 59, jetbrains_mono_76);
      };
    };
  };
}
