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

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Alarm : public Screen {
      public:
        Alarm(DisplayApp* app,
              Controllers::AlarmController& alarmController,
              Pinetime::Controllers::Settings& settingsController,
              System::SystemTask& systemTask);
        ~Alarm() override;
        void SetAlerting();
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        void StopAlerting();

      private:
        uint8_t alarmHours;
        uint8_t alarmMinutes;
        Controllers::AlarmController& alarmController;
        Controllers::Settings& settingsController;
        System::SystemTask& systemTask;

        lv_obj_t *time, *lblampm, *btnStop, *txtStop, *btnMinutesUp, *btnMinutesDown, *btnHoursUp, *btnHoursDown, *txtMinUp,
          *txtMinDown, *txtHrUp, *txtHrDown, *btnRecur, *txtRecur, *btnInfo, *txtInfo, *enableSwitch;
        lv_obj_t* txtMessage = nullptr;
        lv_obj_t* btnMessage = nullptr;
        lv_task_t* taskStopAlarm = nullptr;

        enum class EnableButtonState { On, Off, Alerting };
        void SetRecurButtonState();
        void SetSwitchState(lv_anim_enable_t anim);
        void SetAlarm();
        void ShowInfo();
        void HideInfo();
        void ToggleRecurrence();
        void UpdateAlarmTime();
      };
    };
  };
}
